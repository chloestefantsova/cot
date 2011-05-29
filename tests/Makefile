CPP_SRCS := \
./pair.cpp

OBJS := \
./pair.o

LIBS := 

TEST_LIBS := \
-lcppunit

TEST_SRCS := \
./TestUtils.cpp 

TEST_OBJS := \
./TestUtils.o 

INCLUDE := \
-I ./ \

all: pair cot.test
 

# Tool invocations
	
cot.test: $(TEST_OBJS) $(TEST_LIBS) $(LIBS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++  -o"cot.test" -Wall -pedantic $(TEST_OBJS) $(LIBS) $(INCLUDE) $(TEST_LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	
pair: $(OBJS) $(LIBS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++  -o"pair" -Wall -pedantic $(OBJS) $(LIBS) $(INCLUDE)
	@echo 'Finished building target: $@'
	@echo ' '
		
	
#Each subdirectory must supply rules for building sources it contributes
#src/%.o: ../src/%.cpp
#	@echo 'Building file: $<'
#	@echo 'Invoking: GCC C++ Compiler'
#	g++ -O0 -g3 -Wall -c -fmessage-length=0 -o"$@" "$<" $(INCLUDE)
#	@echo 'Finished building: $<'
#	@echo ' '


clean:
	-$(RM) $(OBJS) $(TEST_OBJS) cot.test
	-@echo ' '
