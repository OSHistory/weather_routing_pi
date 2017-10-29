################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/AboutDialog.cpp \
../src/Boat.cpp \
../src/BoatDialog.cpp \
../src/ConfigurationBatchDialog.cpp \
../src/ConfigurationDialog.cpp \
../src/EditPolarDialog.cpp \
../src/FilterRoutesDialog.cpp \
../src/GribRecord.cpp \
../src/LineBufferOverlay.cpp \
../src/PlotDialog.cpp \
../src/Polar.cpp \
../src/PolygonRegion.cpp \
../src/ReportDialog.cpp \
../src/RouteMap.cpp \
../src/RouteMapOverlay.cpp \
../src/SailboatTransform.cpp \
../src/SettingsDialog.cpp \
../src/StatisticsDialog.cpp \
../src/TexFont.cpp \
../src/Utilities.cpp \
../src/WeatherRouting.cpp \
../src/WeatherRoutingUI.cpp \
../src/icons.cpp \
../src/weather_routing_pi.cpp \
../src/wrdc.cpp \
../src/zuFile.cpp 

C_SRCS += \
../src/georef.c 

OBJS += \
./src/AboutDialog.o \
./src/Boat.o \
./src/BoatDialog.o \
./src/ConfigurationBatchDialog.o \
./src/ConfigurationDialog.o \
./src/EditPolarDialog.o \
./src/FilterRoutesDialog.o \
./src/GribRecord.o \
./src/LineBufferOverlay.o \
./src/PlotDialog.o \
./src/Polar.o \
./src/PolygonRegion.o \
./src/ReportDialog.o \
./src/RouteMap.o \
./src/RouteMapOverlay.o \
./src/SailboatTransform.o \
./src/SettingsDialog.o \
./src/StatisticsDialog.o \
./src/TexFont.o \
./src/Utilities.o \
./src/WeatherRouting.o \
./src/WeatherRoutingUI.o \
./src/georef.o \
./src/icons.o \
./src/weather_routing_pi.o \
./src/wrdc.o \
./src/zuFile.o 

CPP_DEPS += \
./src/AboutDialog.d \
./src/Boat.d \
./src/BoatDialog.d \
./src/ConfigurationBatchDialog.d \
./src/ConfigurationDialog.d \
./src/EditPolarDialog.d \
./src/FilterRoutesDialog.d \
./src/GribRecord.d \
./src/LineBufferOverlay.d \
./src/PlotDialog.d \
./src/Polar.d \
./src/PolygonRegion.d \
./src/ReportDialog.d \
./src/RouteMap.d \
./src/RouteMapOverlay.d \
./src/SailboatTransform.d \
./src/SettingsDialog.d \
./src/StatisticsDialog.d \
./src/TexFont.d \
./src/Utilities.d \
./src/WeatherRouting.d \
./src/WeatherRoutingUI.d \
./src/icons.d \
./src/weather_routing_pi.d \
./src/wrdc.d \
./src/zuFile.d 

C_DEPS += \
./src/georef.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


