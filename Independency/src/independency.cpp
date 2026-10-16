#include <cstdio>
#include <cstdlib>

#include "argh.h"
#include "pugixml.hpp"

#if defined(_WIN32)
#include <windows.h>
#endif

#include "crstl/atomic.h"
#include "crstl/filesystem.h"
#include "crstl/open_hashmap.h"
#include "crstl/path.h"
#include "crstl/process.h"
#include "crstl/timer.h"
#include "crstl/thread.h"
#include "crstl/vector.h"

struct Repository
{
	crstl::string name; // Name of this repository

	crstl::string url; // URL of this repository
};

struct Version
{
	crstl::string repositoryName; // Where to find this version

	crstl::string id; // Version id of package

	crstl::string filename; // Zip where it's contained
};

struct Package
{
	crstl::string name; // Package name

	crstl::open_hashmap<crstl::string, Version> versions; // Available versions of the package

	crstl::string ToString() const
	{
		crstl::string s;
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
	crstl::string name; // Name of the package

	crstl::string versionId; // Version we depend on
};

crstl::path CurrentExecutableDirectory;

const crstl::path& GetCurrentExecutableDirectory()
{
	return CurrentExecutableDirectory;
}

struct SystemCommand
{
	crstl::string executable;
	crstl::string args;
	crstl::string currentDirectory;
};

bool RunSystemCommand(const SystemCommand& command, crstl::string& output)
{
	output.clear();

	crstl::process systemCommand(command.executable.c_str(), command.args.c_str());

	if (systemCommand.is_launched())
	{
		systemCommand.drain_stdout([&output](const char* read_data, size_t read_size)
		{
			output.append(read_data, read_size);
		});

		// The process returns 0 if it manages to run, but we're looking for errors in the execution
		// such as a URL that wasn't found. The ERROR string seems to work for both wget and 7zip
		if (output.find("ERROR") != crstl::string::npos)
		{
			return false;
		}

		crstl::process_exit_status exit_code = systemCommand.wait();
		if (exit_code)
		{
			int return_code = exit_code.get_exit_code();
			if (return_code != -1)
			{
				return true;
			}
		}
	}

	return false;
}

bool UnzipFile(const crstl::path& zippedPath, const crstl::path& unzipDirectory, crstl::string& output)
{
	SystemCommand systemCommand;
	systemCommand.executable = (GetCurrentExecutableDirectory() / "tools/7za.exe").c_str();
	systemCommand.args.append("x ");
	systemCommand.args.append(zippedPath.c_str());
	systemCommand.args.append(" -o");
	systemCommand.args.append(unzipDirectory.c_str());
	systemCommand.args.append(" -y");
	
	return RunSystemCommand(systemCommand, output);
}

bool DownloadFile(const crstl::string& repositoryURL, const crstl::string& filename, const crstl::path& destinationDirectory, crstl::string& output)
{
	crstl::string finalUrl = repositoryURL + filename;

	SystemCommand systemCommand;
	systemCommand.executable = (GetCurrentExecutableDirectory() / "tools/wget.exe").c_str();
	systemCommand.args = finalUrl + " --no-cache --no-hsts -N -P " + destinationDirectory.c_str();

	return RunSystemCommand(systemCommand, output);
}

int main(int argc, char* argv[])
{
	CurrentExecutableDirectory = argv[0];
	CurrentExecutableDirectory.remove_filename();

	argh::parser commandline(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);

	crstl::path dependenciesCommandLine    = commandline("-dependencies").str().c_str();
	crstl::path packagesCommandLine        = commandline("-packages").str().c_str();
	crstl::path destinationPathCommandLine = commandline("-destination").str().c_str();
	bool multithreaded                     = !commandline["-singlethreaded"];

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

	crstl::string dependenciesFilename;
	crstl::string packagesFilename;
	crstl::path dependenciesPath;

	if (dependenciesCommandLine.is_absolute())
	{
		dependenciesFilename = dependenciesCommandLine.c_str();
	}
	else
	{
		dependenciesFilename = (crstl::current_directory_path() / dependenciesCommandLine).c_str();
	}

	if (packagesCommandLine.is_absolute())
	{
		packagesFilename = packagesCommandLine.c_str();
	}
	else
	{
		packagesFilename = (crstl::current_directory_path() / packagesCommandLine).c_str();
	}

	if (destinationPathCommandLine.is_absolute())
	{
		dependenciesPath = destinationPathCommandLine;
	}
	else
	{
		dependenciesPath = (crstl::current_directory_path() / destinationPathCommandLine);
	}

	//std::string downloadPath = (fs::temp_directory_path() / "Independency/Downloads").string();
	crstl::path downloadPath = crstl::temp_directory_path() / "Independency/Downloads";

	crstl::open_hashmap<crstl::string, Package> packages;
	crstl::open_hashmap<crstl::string, Repository> repositories;
	crstl::vector<Dependency> dependencies;

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
			Repository repository;
			repository.name = repositoryNode.attribute("name").as_string();
			repository.url = repositoryNode.attribute("url").as_string();
			repositories.insert(repository.name, repository);
		}

		for (pugi::xml_node packageNode : packagesNode.children("package"))
		{
			Package package;
			package.name = packageNode.attribute("name").as_string();
			for (pugi::xml_node versionNode : packageNode)
			{
				Version version;
				version.id = versionNode.attribute("id").as_string();
				version.filename = versionNode.attribute("file").as_string();
				version.repositoryName = versionNode.attribute("repository").as_string();
				package.versions.insert(version.id, version);
			}

			packages.insert(package.name, package);
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
			dependencies.push_back(dependency);
		}
	}

	uint32_t dependencyCount = (uint32_t)dependencies.size();
	crstl::atomic<uint32_t> currentProcessedDependency = 1;

	crstl::vector<crstl::thread> threads;
	threads.resize(dependencyCount);

	const auto ProcessDependency = [&](const Dependency& dependency)
	{
		uint32_t currentDependencyIndex = currentProcessedDependency++;

		printf("Started %20s %i/%i\n", dependency.name.c_str(), currentDependencyIndex, dependencyCount);

		bool successful = false;

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

					crstl::string downloadOutput;
					if (DownloadFile(repository.url, version.filename, downloadPath, downloadOutput))
					{
						crstl::string unzipOutput;
						if (UnzipFile(downloadPath / version.filename, dependenciesPath / package.name, unzipOutput))
						{
							successful = true;
						}
						else
						{
							printf("%s", unzipOutput.c_str());
						}
					}
					else
					{
						printf("%s", downloadOutput.c_str());
					}
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

		if (successful)
		{
			printf("Successfully processed %20s %i/%i\n", dependency.name.c_str(), currentDependencyIndex, dependencyCount);
		}
		else
		{
			printf("Failed to process %20s %i/%i\n\n", dependency.name.c_str(), currentDependencyIndex, dependencyCount);
		}
	};

	size_t currentThread = 0;

	// Download required dependencies to target directory
	crstl::timer timer;
	{
		for (const Dependency& dependency : dependencies)
		{
			if (multithreaded)
			{
				crstl::thread_parameters params;
				threads[currentThread] = crstl::thread(params, ProcessDependency, dependency);
				currentThread++;
			}
			else
			{
				ProcessDependency(dependency);
			}
		}
	}

	if (multithreaded)
	{
		for (size_t i = 0; i < threads.size(); ++i)
		{
			threads[i].join();
		}
	}

	double elapsedTimeSeconds = timer.elapsed().seconds();

	printf("Finished downloading dependencies (%f seconds)\n", elapsedTimeSeconds);
}