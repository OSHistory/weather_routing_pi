################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/libtess2/bucketalloc.c \
../src/libtess2/dict.c \
../src/libtess2/geom.c \
../src/libtess2/mesh.c \
../src/libtess2/priorityq.c \
../src/libtess2/sweep.c \
../src/libtess2/tess.c 

OBJS += \
./src/libtess2/bucketalloc.o \
./src/libtess2/dict.o \
./src/libtess2/geom.o \
./src/libtess2/mesh.o \
./src/libtess2/priorityq.o \
./src/libtess2/sweep.o \
./src/libtess2/tess.o 

C_DEPS += \
./src/libtess2/bucketalloc.d \
./src/libtess2/dict.d \
./src/libtess2/geom.d \
./src/libtess2/mesh.d \
./src/libtess2/priorityq.d \
./src/libtess2/sweep.d \
./src/libtess2/tess.d 


# Each subdirectory must supply rules for building sources it contributes
src/libtess2/%.o: ../src/libtess2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


