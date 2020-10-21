#include <fstream>
#include <sstream>
#include "OpenCLHelper.h"

OpenCLHelper::OpenCLHelper(const std::string &file) {
	createProgram(file);
}

void OpenCLHelper::createProgram(const std::string &file) {
	int err = CL_SUCCESS;

	std::vector<cl::Platform> platforms;
	err = cl::Platform::get(&platforms);
	std::cout << err << std::endl;
	if (platforms.empty()) {
		std::cout << "Unable to find suitable platform." << std::endl;
	}

	cl_context_properties properties[] =
			{CL_CONTEXT_PLATFORM, (cl_context_properties) (platforms[0])(), 0};
	cl::Context _context(CL_DEVICE_TYPE_GPU, properties);

	std::vector<cl::Device> _devices = _context.getInfo<CL_CONTEXT_DEVICES>();

	std::string programSource = FileToString(file);
	cl::Program _program = cl::Program(_context, programSource, false, &err);
	std::cout << err << std::endl;
	err = _program.build(_devices);
	std::cout << err << std::endl;

	this->program = _program;
	this->context = program.getInfo<CL_PROGRAM_CONTEXT>();
	this->devices = context.getInfo<CL_CONTEXT_DEVICES>();
	this->device = devices.front();
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