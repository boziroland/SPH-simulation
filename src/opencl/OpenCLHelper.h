#ifndef OPENCLHELPER_H
#define OPENCLHELPER_H

#include "opencl.hpp"
#include <iostream>

class OpenCLHelper {
private:
	cl::Program program;
	cl::Context context;
	std::vector<cl::Device> devices;
	cl::Device device;

	void createProgram(const std::string &file);
	static std::string FileToString(const std::string &path);
	void getError(const cl::Program&, int err);
	static auto GetSource(std::string const& fileName);
public:
	explicit OpenCLHelper(const std::string& file);
	cl::Program& getProgram();
	cl::Context& getContext();
	std::vector<cl::Device>& getDevices();
	cl::Device& getDevice();
};
#endif //TESTOPENCL_OPENCLHELPER_H
