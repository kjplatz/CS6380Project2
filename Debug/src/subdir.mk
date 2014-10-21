################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Message.cpp \
../src/main.cpp \
../src/parse_config.cpp \
../src/run_node.cpp 

OBJS += \
./src/Message.o \
./src/main.o \
./src/parse_config.o \
./src/run_node.o 

CPP_DEPS += \
./src/Message.d \
./src/main.d \
./src/parse_config.d \
./src/run_node.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -O0 -g3 -pedantic -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


