#include <fstream>
#include <sstream>
#include "OpenCLHelper.h"
#include "../Constants.h"
#include "../ParticleData.h"
#include "../Particle.h"

//#define CL_HPP_ENABLE_EXCEPTIONS

OpenCLHelper::OpenCLHelper(const std::string &file, const std::vector<Particle>& hostBuffer) {
	createProgram(file, hostBuffer);
}

auto OpenCLHelper::GetSource(std::string const &fileName) {
	std::ifstream f{fileName};
	if (!f.is_open()) {
		std::cout << "Cannot open file: " << fileName << std::endl;
		throw std::runtime_error{"Cannot open file"};
	}
	return std::string{std::istreambuf_iterator<char>{f}, std::istreambuf_iterator<char>{}};
}

void OpenCLHelper::createProgram(const std::string &file, const std::vector<Particle>& hostBuffer) {
	try {
		int err = CL_SUCCESS;

		std::vector<cl::Platform> platforms;
		err = cl::Platform::get(&platforms);
		std::cout << err << std::endl;
		if (platforms.empty()) {
			std::cout << "Unable to find suitable platform." << std::endl;
		}else{
			std::cout << "Platform count : " << platforms.size() << std::endl;
		}

		cl_context_properties properties[] =
				{CL_CONTEXT_PLATFORM, (cl_context_properties) (platforms[0])(), 0};
		cl::Context _context(CL_DEVICE_TYPE_GPU, properties);

		std::vector<cl::Device> _devices = _context.getInfo<CL_CONTEXT_DEVICES>();
		std::cout << "DEVICES : " << _devices.size() << std::endl;
		for(const auto& dev : _devices) {
			std::cout << "NAME : " << dev.getInfo<CL_DEVICE_NAME>() << std::endl;
		}

		auto programSource = GetSource(file);
		cl::Program _program = cl::Program(_context, programSource, false, &err);

		getError(_program, err);
		std::cout << err << std::endl;

		err = _program.build(_devices);
		getError(_program, err);
		std::cout << err << std::endl;

		this->program = _program;
		this->context = program.getInfo<CL_PROGRAM_CONTEXT>();
		this->devices = context.getInfo<CL_CONTEXT_DEVICES>();
		this->device = devices.front();

		cl::Kernel kernelUpdate{program, "updateForces", &err};
		cl::Kernel kernelMove{program, "updatePosition", &err};

		int bufferSize = particle_cols_amount * particle_rows_amount;

		cl::Buffer clInputBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(ParticleData) * bufferSize, NULL,
											  &err);
		// << getErrorString(err) << std::endl;
		cl::Buffer clCountBuffer = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(int), NULL, &err);
		//std::cout << getErrorString(err) << std::endl;

		err = kernelUpdate.setArg(0, clInputBuffer);
		//std::cout << getErrorString(err) << std::endl;
		err = kernelUpdate.setArg(1, clCountBuffer);

		err = kernelMove.setArg(0, clInputBuffer);
		//std::cout << getErrorString(err) << std::endl;
		err = kernelMove.setArg(1, clCountBuffer);
		//std::cout << getErrorString(err) << std::endl;


		this->moveKernel = kernelMove;
		this->updateKernel = kernelUpdate;
		this->inputBuffer = clInputBuffer;
		this->countBuffer = clCountBuffer;
		//this->queue = myQueue;

		getError(program, err);
	} catch (cl::Error &e) {
		std::cout << "cl error was thrown" << std::endl;
		std::cout << e.what() << " : " << e.err();
//		if (e.err() == CL_BUILD_PROGRAM_FAILURE) {
//			for (cl::Device dev : devices) {
//				// Check the build status
//				cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
//				//if (status != CL_BUILD_ERROR)
//				//	continue;
//
//				// Get the build log
//				std::string name = dev.getInfo<CL_DEVICE_NAME>();
//				std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
//				std::cerr << "Build log for " << name << ":" << std::endl
//						  << buildlog << std::endl;
//			}
//		}
	}
}

void OpenCLHelper::getError(const cl::Program &_program, int err) {
	if (err != 0)
		std::cout << _program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
}

std::string OpenCLHelper::FileToString(const std::string &path) {
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (file) {
		std::ostringstream contents;
		contents << file.rdbuf();
		file.close();
		return (contents.str());
	}
	return std::string();
}

cl::Program &OpenCLHelper::getProgram() {
	return program;
}

cl::Context &OpenCLHelper::getContext() {
	return context;
}

std::vector<cl::Device> &OpenCLHelper::getDevices() {
	return devices;
}

cl::Device &OpenCLHelper::getDevice() {
	return device;
}