################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bzip2/blocksort.c \
../src/bzip2/bzlib.c \
../src/bzip2/compress.c \
../src/bzip2/crctable.c \
../src/bzip2/decompress.c \
../src/bzip2/huffman.c \
../src/bzip2/randtable.c 

OBJS += \
./src/bzip2/blocksort.o \
./src/bzip2/bzlib.o \
./src/bzip2/compress.o \
./src/bzip2/crctable.o \
./src/bzip2/decompress.o \
./src/bzip2/huffman.o \
./src/bzip2/randtable.o 

C_DEPS += \
./src/bzip2/blocksort.d \
./src/bzip2/bzlib.d \
./src/bzip2/compress.d \
./src/bzip2/crctable.d \
./src/bzip2/decompress.d \
./src/bzip2/huffman.d \
./src/bzip2/randtable.d 


# Each subdirectory must supply rules for building sources it contributes
src/bzip2/%.o: ../src/bzip2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


