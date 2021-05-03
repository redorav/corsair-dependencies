#include <string>
#include <filesystem>

#include <vector>
#include <unordered_map>

#include <cstdio>
#include <cstdlib>

#include "argh.h"
#include "pugixml.hpp"
#include "subprocess.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace fs = std::filesystem;
using path = std::filesystem::path;

struct Repository
{
	std::string name; // Name of this repository

	std::string url; // URL of this repository
};

struct Version
{
	std::string repositoryName; // Where to find this version

	std::string id; // Version id of package

	std::string filename; // Zip where it's contained
};

struct Package
{
	std::string name; // Package name

	std::unordered_map<std::string, Version> versions; // Available versions of the package

	std::string ToString() const
	{
		std::string s;
		s = name + "\n";
		for (const auto& [id, v] : versions)
		{
			s += v.id + " : " + v.filename + "\n";
		}

		return s;
	}
};

struct Dependency
{
	std::string name; // Name of the package

	std::string versionId; // Version we depend on
};

path CurrentExecutableDirectory;

const path& GetCurrentExecutableDirectory()
{
	return CurrentExecutableDirectory;
}

struct SystemCommand
{
	std::string executable;
	std::string args;
	std::string currentDirectory;
};

std::string RunSystemCommand(const SystemCommand& command)
{
	// Print command line
	// printf("Download command line: %s %s\n", command.executable.c_str(), command.args.c_str());

	const char* command_line[] = { command.executable.c_str(), command.args.c_str(), nullptr };

	struct subprocess_s subprocess;
	int result = subprocess_create(command_line, subprocess_option_combined_stdout_stderr | subprocess_option_inherit_environment, &subprocess);

	if (result != 0)
	{
		return "Process launch failed";
	}

	//int process_return;
	//int resultJoin = subprocess_join(&subprocess, &process_return);
	//if (resultJoin != 0)
	//{
	//	return "Process wait failed";
	//}

	FILE* p_stdout = subprocess_stdout(&subprocess);

	std::string data;
	char outputBuffer[1024];

	// https://www.jeremymorgan.com/tutorials/c-programming/how-to-capture-the-output-of-a-linux-command-in-c/
	while (!feof(p_stdout))
	{
		if (fgets(outputBuffer, 1024, p_stdout) != nullptr)
		{
			data.append(outputBuffer);
		}
	}

	// Print output from process
	// printf(data.c_str());

	return data;
}

void UnzipFile(const std::string& zippedPath, const std::string& unzipDirectory)
{
	SystemCommand systemCommand;
	systemCommand.executable = (GetCurrentExecutableDirectory() / "tools/7za.exe").string();
	systemCommand.args = "x " + zippedPath + " -o" + unzipDirectory + " -y";

	RunSystemCommand(systemCommand);
}

void DownloadFile(const std::string& repositoryURL, const std::string& filename, const path& destinationDirectory)
{
	std::string finalUrl = repositoryURL + filename;

	SystemCommand systemCommand;
	systemCommand.executable = (GetCurrentExecutableDirectory() / "tools/aria2c.exe").string();
	systemCommand.args = "-c --conditional-get=true --auto-file-renaming=false --dir=" + destinationDirectory.string() + " --out=" + filename + " " + finalUrl;

	RunSystemCommand(systemCommand);
}

int main(int argc, char* argv[])
{
	CurrentExecutableDirectory = argv[0];
	CurrentExecutableDirectory.remove_filename();

	argh::parser commandline(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	path dependenciesCommandLine    = commandline("-dependencies").str();
	path packagesCommandLine        = commandline("-packages").str();
	path destinationPathCommandLine = commandline("-destination").str();

	bool error = false;

	if (dependenciesCommandLine.empty())
	{
		printf("Dependencies file not specified\n");
		error = true;
	}

	if (packagesCommandLine.empty())
	{
		printf("Packages file not specified\n");
		error = true;
	}

	if (destinationPathCommandLine.empty())
	{
		printf("Destination not specified\n");
		error = true;
	}

	if (error)
	{
		return -1;
	}

	std::string dependenciesFilename;
	std::string packagesFilename;
	std::string dependenciesPath;

	if (dependenciesCommandLine.is_absolute())
	{
		dependenciesFilename = dependenciesCommandLine.string();
	}
	else
	{
		dependenciesFilename = (fs::current_path() / dependenciesCommandLine).string();
	}

	if (packagesCommandLine.is_absolute())
	{
		packagesFilename = packagesCommandLine.string();
	}
	else
	{
		packagesFilename = (fs::current_path() / packagesCommandLine).string();
	}

	if (destinationPathCommandLine.is_absolute())
	{
		dependenciesPath = destinationPathCommandLine.string();
	}
	else
	{
		dependenciesPath = (fs::current_path() / destinationPathCommandLine).string();
	}

	std::string downloadPath = (fs::temp_directory_path() / "Independency/Downloads").string();

	std::unordered_map<std::string, Package> packages;
	std::unordered_map<std::string, Dependency> dependencies;
	std::unordered_map<std::string, Repository> repositories;

	// Parse Packages
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(packagesFilename.c_str());

		if (!result)
		{
			return -1;
		}
		
		pugi::xml_node packagesNode = doc.child("packages");

		for (pugi::xml_node repositoryNode : packagesNode.children("repository"))
		{
			struct Repository repository;
			repository.name = repositoryNode.attribute("name").as_string();
			repository.url = repositoryNode.attribute("url").as_string();
			repositories.insert({ repository.name, repository });
		}

		for (pugi::xml_node packageNode : packagesNode.children("package"))
		{
			struct Package package;
			package.name = packageNode.attribute("name").as_string();
			for (pugi::xml_node versionNode : packageNode)
			{
				Version version;
				version.id = versionNode.attribute("id").as_string();
				version.filename = versionNode.attribute("file").as_string();
				version.repositoryName = versionNode.attribute("repository").as_string();
				package.versions.insert({ version.id, version });
			}

			packages.insert({ package.name, package });
		}
	}
	
	// Parse dependencies
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(dependenciesFilename.c_str());

		if (!result)
		{
			return -1;
		}

		for (pugi::xml_node dependencyNode : doc.child("dependencies"))
		{
			struct Dependency dependency;
			dependency.name = dependencyNode.attribute("name").as_string();
			dependency.versionId = dependencyNode.attribute("version").as_string();
			dependencies.insert({ dependency.name, dependency });
		}
	}

	uint32_t dependencyCount = (uint32_t)dependencies.size();
	uint32_t currentProcessedDependency = 1;

	// Download required dependencies to target directory
	{
		for (const auto& [name, dependency] : dependencies)
		{
			printf("Processing %20s : %i/%i\n", name.c_str(), currentProcessedDependency, dependencyCount);

			const auto& packageIter = packages.find(dependency.name);
			if (packageIter != packages.end())
			{
				const Package& package = packageIter->second;
				const auto& versionIter = package.versions.find(dependency.versionId);
				if (versionIter != package.versions.end())
				{
					const Version& version = versionIter->second;
					const auto& repositoryIter = repositories.find(version.repositoryName);

					if (repositoryIter != repositories.end())
					{
						const Repository& repository = repositoryIter->second;

						DownloadFile(repository.url, version.filename, downloadPath);

						UnzipFile(downloadPath + "/" + version.filename, dependenciesPath + "/" + package.name);
					}
					else
					{
						printf("Repository %s not found", version.repositoryName.c_str());
					}
				}
				else
				{
					printf("Package version %s not found", dependency.versionId.c_str());
				}
			}
			else
			{
				printf("Package %s not found", dependency.name.c_str());
			}

			currentProcessedDependency++;
		}
	}

	printf("Finished downloading dependencies\n");
}