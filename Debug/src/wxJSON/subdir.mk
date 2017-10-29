################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/wxJSON/jsonreader.cpp \
../src/wxJSON/jsonval.cpp \
../src/wxJSON/jsonwriter.cpp 

OBJS += \
./src/wxJSON/jsonreader.o \
./src/wxJSON/jsonval.o \
./src/wxJSON/jsonwriter.o 

CPP_DEPS += \
./src/wxJSON/jsonreader.d \
./src/wxJSON/jsonval.d \
./src/wxJSON/jsonwriter.d 


# Each subdirectory must supply rules for building sources it contributes
src/wxJSON/%.o: ../src/wxJSON/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


