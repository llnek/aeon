##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=cpp
ConfigurationName      :=Debug
WorkspacePath          :=/Users/kenl/wdrive/mygit/rygel/src/main/cpp
ProjectPath            :=/Users/kenl/wdrive/mygit/rygel/src/main/cpp
IntermediateDirectory  :=$(ConfigurationName)
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Kenneth Leung
Date                   :=15/07/2021
CodeLitePath           :="/Users/kenl/Library/Application Support/CodeLite"
LinkerName             :=/usr/bin/clang++
SharedObjectLinkerName :=/usr/bin/clang++ -dynamiclib -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="cpp.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -O0
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch). $(LibraryPathSwitch)Debug 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/clang++
CC       := /usr/bin/clang
CXXFLAGS :=  -std=c++17 -std=c++14 -std=c++11 -g -Wall $(Preprocessors)
CFLAGS   :=   $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/Applications/codelite.app/Contents/SharedSupport/
Objects0=$(IntermediateDirectory)/elle_types.cpp$(ObjectSuffix) $(IntermediateDirectory)/elle_builtins.cpp$(ObjectSuffix) $(IntermediateDirectory)/elle_parser.cpp$(ObjectSuffix) $(IntermediateDirectory)/tiny14e_types.cpp$(ObjectSuffix) $(IntermediateDirectory)/tiny14e_interpreter.cpp$(ObjectSuffix) $(IntermediateDirectory)/tiny14e_test.cpp$(ObjectSuffix) $(IntermediateDirectory)/ecs_engine.cpp$(ObjectSuffix) $(IntermediateDirectory)/basic_types.cpp$(ObjectSuffix) $(IntermediateDirectory)/otto_reader.cpp$(ObjectSuffix) $(IntermediateDirectory)/basic_basic.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/basic_test.cpp$(ObjectSuffix) $(IntermediateDirectory)/ecs_types.cpp$(ObjectSuffix) $(IntermediateDirectory)/ecs_node.cpp$(ObjectSuffix) $(IntermediateDirectory)/spi_test.cpp$(ObjectSuffix) $(IntermediateDirectory)/elle_schemer.cpp$(ObjectSuffix) $(IntermediateDirectory)/basic_lexer.cpp$(ObjectSuffix) $(IntermediateDirectory)/spi_lexer.cpp$(ObjectSuffix) $(IntermediateDirectory)/tiny14e_parser.cpp$(ObjectSuffix) $(IntermediateDirectory)/spi_interpreter.cpp$(ObjectSuffix) $(IntermediateDirectory)/spi_parser.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/aeon_test.cpp$(ObjectSuffix) $(IntermediateDirectory)/elle_test.cpp$(ObjectSuffix) $(IntermediateDirectory)/aeon_pool.cpp$(ObjectSuffix) $(IntermediateDirectory)/otto_types.cpp$(ObjectSuffix) $(IntermediateDirectory)/otto_builtins.cpp$(ObjectSuffix) $(IntermediateDirectory)/basic_builtins.cpp$(ObjectSuffix) $(IntermediateDirectory)/basic_parser.cpp$(ObjectSuffix) $(IntermediateDirectory)/otto_lisper.cpp$(ObjectSuffix) $(IntermediateDirectory)/otto_parser.cpp$(ObjectSuffix) $(IntermediateDirectory)/elle_reader.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/ecs_test.cpp$(ObjectSuffix) $(IntermediateDirectory)/aeon_aeon.cpp$(ObjectSuffix) $(IntermediateDirectory)/tiny14e_lexer.cpp$(ObjectSuffix) $(IntermediateDirectory)/dsl_dsl.cpp$(ObjectSuffix) $(IntermediateDirectory)/otto_test.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)


$(IntermediateDirectory)/.d:
	@test -d $(ConfigurationName) || $(MakeDirCommand) $(ConfigurationName)

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/elle_types.cpp$(ObjectSuffix): elle/types.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/elle_types.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/elle_types.cpp$(DependSuffix) -MM elle/types.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/elle/types.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/elle_types.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/elle_types.cpp$(PreprocessSuffix): elle/types.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/elle_types.cpp$(PreprocessSuffix) elle/types.cpp

$(IntermediateDirectory)/elle_builtins.cpp$(ObjectSuffix): elle/builtins.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/elle_builtins.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/elle_builtins.cpp$(DependSuffix) -MM elle/builtins.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/elle/builtins.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/elle_builtins.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/elle_builtins.cpp$(PreprocessSuffix): elle/builtins.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/elle_builtins.cpp$(PreprocessSuffix) elle/builtins.cpp

$(IntermediateDirectory)/elle_parser.cpp$(ObjectSuffix): elle/parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/elle_parser.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/elle_parser.cpp$(DependSuffix) -MM elle/parser.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/elle/parser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/elle_parser.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/elle_parser.cpp$(PreprocessSuffix): elle/parser.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/elle_parser.cpp$(PreprocessSuffix) elle/parser.cpp

$(IntermediateDirectory)/tiny14e_types.cpp$(ObjectSuffix): tiny14e/types.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/tiny14e_types.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/tiny14e_types.cpp$(DependSuffix) -MM tiny14e/types.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/tiny14e/types.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/tiny14e_types.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tiny14e_types.cpp$(PreprocessSuffix): tiny14e/types.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/tiny14e_types.cpp$(PreprocessSuffix) tiny14e/types.cpp

$(IntermediateDirectory)/tiny14e_interpreter.cpp$(ObjectSuffix): tiny14e/interpreter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/tiny14e_interpreter.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/tiny14e_interpreter.cpp$(DependSuffix) -MM tiny14e/interpreter.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/tiny14e/interpreter.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/tiny14e_interpreter.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tiny14e_interpreter.cpp$(PreprocessSuffix): tiny14e/interpreter.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/tiny14e_interpreter.cpp$(PreprocessSuffix) tiny14e/interpreter.cpp

$(IntermediateDirectory)/tiny14e_test.cpp$(ObjectSuffix): tiny14e/test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/tiny14e_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/tiny14e_test.cpp$(DependSuffix) -MM tiny14e/test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/tiny14e/test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/tiny14e_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tiny14e_test.cpp$(PreprocessSuffix): tiny14e/test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/tiny14e_test.cpp$(PreprocessSuffix) tiny14e/test.cpp

$(IntermediateDirectory)/ecs_engine.cpp$(ObjectSuffix): ecs/engine.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ecs_engine.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ecs_engine.cpp$(DependSuffix) -MM ecs/engine.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/ecs/engine.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ecs_engine.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ecs_engine.cpp$(PreprocessSuffix): ecs/engine.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ecs_engine.cpp$(PreprocessSuffix) ecs/engine.cpp

$(IntermediateDirectory)/basic_types.cpp$(ObjectSuffix): basic/types.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basic_types.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/basic_types.cpp$(DependSuffix) -MM basic/types.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/basic/types.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basic_types.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basic_types.cpp$(PreprocessSuffix): basic/types.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basic_types.cpp$(PreprocessSuffix) basic/types.cpp

$(IntermediateDirectory)/otto_reader.cpp$(ObjectSuffix): otto/reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/otto_reader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/otto_reader.cpp$(DependSuffix) -MM otto/reader.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/otto/reader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/otto_reader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/otto_reader.cpp$(PreprocessSuffix): otto/reader.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/otto_reader.cpp$(PreprocessSuffix) otto/reader.cpp

$(IntermediateDirectory)/basic_basic.cpp$(ObjectSuffix): basic/basic.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basic_basic.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/basic_basic.cpp$(DependSuffix) -MM basic/basic.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/basic/basic.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basic_basic.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basic_basic.cpp$(PreprocessSuffix): basic/basic.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basic_basic.cpp$(PreprocessSuffix) basic/basic.cpp

$(IntermediateDirectory)/basic_test.cpp$(ObjectSuffix): basic/test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basic_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/basic_test.cpp$(DependSuffix) -MM basic/test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/basic/test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basic_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basic_test.cpp$(PreprocessSuffix): basic/test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basic_test.cpp$(PreprocessSuffix) basic/test.cpp

$(IntermediateDirectory)/ecs_types.cpp$(ObjectSuffix): ecs/types.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ecs_types.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ecs_types.cpp$(DependSuffix) -MM ecs/types.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/ecs/types.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ecs_types.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ecs_types.cpp$(PreprocessSuffix): ecs/types.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ecs_types.cpp$(PreprocessSuffix) ecs/types.cpp

$(IntermediateDirectory)/ecs_node.cpp$(ObjectSuffix): ecs/node.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ecs_node.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ecs_node.cpp$(DependSuffix) -MM ecs/node.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/ecs/node.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ecs_node.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ecs_node.cpp$(PreprocessSuffix): ecs/node.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ecs_node.cpp$(PreprocessSuffix) ecs/node.cpp

$(IntermediateDirectory)/spi_test.cpp$(ObjectSuffix): spi/test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/spi_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/spi_test.cpp$(DependSuffix) -MM spi/test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/spi/test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/spi_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/spi_test.cpp$(PreprocessSuffix): spi/test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/spi_test.cpp$(PreprocessSuffix) spi/test.cpp

$(IntermediateDirectory)/elle_schemer.cpp$(ObjectSuffix): elle/schemer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/elle_schemer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/elle_schemer.cpp$(DependSuffix) -MM elle/schemer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/elle/schemer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/elle_schemer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/elle_schemer.cpp$(PreprocessSuffix): elle/schemer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/elle_schemer.cpp$(PreprocessSuffix) elle/schemer.cpp

$(IntermediateDirectory)/basic_lexer.cpp$(ObjectSuffix): basic/lexer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basic_lexer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/basic_lexer.cpp$(DependSuffix) -MM basic/lexer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/basic/lexer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basic_lexer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basic_lexer.cpp$(PreprocessSuffix): basic/lexer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basic_lexer.cpp$(PreprocessSuffix) basic/lexer.cpp

$(IntermediateDirectory)/spi_lexer.cpp$(ObjectSuffix): spi/lexer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/spi_lexer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/spi_lexer.cpp$(DependSuffix) -MM spi/lexer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/spi/lexer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/spi_lexer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/spi_lexer.cpp$(PreprocessSuffix): spi/lexer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/spi_lexer.cpp$(PreprocessSuffix) spi/lexer.cpp

$(IntermediateDirectory)/tiny14e_parser.cpp$(ObjectSuffix): tiny14e/parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/tiny14e_parser.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/tiny14e_parser.cpp$(DependSuffix) -MM tiny14e/parser.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/tiny14e/parser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/tiny14e_parser.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tiny14e_parser.cpp$(PreprocessSuffix): tiny14e/parser.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/tiny14e_parser.cpp$(PreprocessSuffix) tiny14e/parser.cpp

$(IntermediateDirectory)/spi_interpreter.cpp$(ObjectSuffix): spi/interpreter.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/spi_interpreter.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/spi_interpreter.cpp$(DependSuffix) -MM spi/interpreter.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/spi/interpreter.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/spi_interpreter.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/spi_interpreter.cpp$(PreprocessSuffix): spi/interpreter.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/spi_interpreter.cpp$(PreprocessSuffix) spi/interpreter.cpp

$(IntermediateDirectory)/spi_parser.cpp$(ObjectSuffix): spi/parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/spi_parser.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/spi_parser.cpp$(DependSuffix) -MM spi/parser.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/spi/parser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/spi_parser.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/spi_parser.cpp$(PreprocessSuffix): spi/parser.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/spi_parser.cpp$(PreprocessSuffix) spi/parser.cpp

$(IntermediateDirectory)/aeon_test.cpp$(ObjectSuffix): aeon/test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/aeon_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/aeon_test.cpp$(DependSuffix) -MM aeon/test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/aeon/test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/aeon_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/aeon_test.cpp$(PreprocessSuffix): aeon/test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/aeon_test.cpp$(PreprocessSuffix) aeon/test.cpp

$(IntermediateDirectory)/elle_test.cpp$(ObjectSuffix): elle/test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/elle_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/elle_test.cpp$(DependSuffix) -MM elle/test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/elle/test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/elle_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/elle_test.cpp$(PreprocessSuffix): elle/test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/elle_test.cpp$(PreprocessSuffix) elle/test.cpp

$(IntermediateDirectory)/aeon_pool.cpp$(ObjectSuffix): aeon/pool.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/aeon_pool.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/aeon_pool.cpp$(DependSuffix) -MM aeon/pool.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/aeon/pool.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/aeon_pool.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/aeon_pool.cpp$(PreprocessSuffix): aeon/pool.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/aeon_pool.cpp$(PreprocessSuffix) aeon/pool.cpp

$(IntermediateDirectory)/otto_types.cpp$(ObjectSuffix): otto/types.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/otto_types.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/otto_types.cpp$(DependSuffix) -MM otto/types.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/otto/types.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/otto_types.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/otto_types.cpp$(PreprocessSuffix): otto/types.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/otto_types.cpp$(PreprocessSuffix) otto/types.cpp

$(IntermediateDirectory)/otto_builtins.cpp$(ObjectSuffix): otto/builtins.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/otto_builtins.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/otto_builtins.cpp$(DependSuffix) -MM otto/builtins.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/otto/builtins.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/otto_builtins.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/otto_builtins.cpp$(PreprocessSuffix): otto/builtins.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/otto_builtins.cpp$(PreprocessSuffix) otto/builtins.cpp

$(IntermediateDirectory)/basic_builtins.cpp$(ObjectSuffix): basic/builtins.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basic_builtins.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/basic_builtins.cpp$(DependSuffix) -MM basic/builtins.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/basic/builtins.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basic_builtins.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basic_builtins.cpp$(PreprocessSuffix): basic/builtins.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basic_builtins.cpp$(PreprocessSuffix) basic/builtins.cpp

$(IntermediateDirectory)/basic_parser.cpp$(ObjectSuffix): basic/parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/basic_parser.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/basic_parser.cpp$(DependSuffix) -MM basic/parser.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/basic/parser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/basic_parser.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/basic_parser.cpp$(PreprocessSuffix): basic/parser.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/basic_parser.cpp$(PreprocessSuffix) basic/parser.cpp

$(IntermediateDirectory)/otto_lisper.cpp$(ObjectSuffix): otto/lisper.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/otto_lisper.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/otto_lisper.cpp$(DependSuffix) -MM otto/lisper.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/otto/lisper.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/otto_lisper.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/otto_lisper.cpp$(PreprocessSuffix): otto/lisper.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/otto_lisper.cpp$(PreprocessSuffix) otto/lisper.cpp

$(IntermediateDirectory)/otto_parser.cpp$(ObjectSuffix): otto/parser.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/otto_parser.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/otto_parser.cpp$(DependSuffix) -MM otto/parser.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/otto/parser.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/otto_parser.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/otto_parser.cpp$(PreprocessSuffix): otto/parser.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/otto_parser.cpp$(PreprocessSuffix) otto/parser.cpp

$(IntermediateDirectory)/elle_reader.cpp$(ObjectSuffix): elle/reader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/elle_reader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/elle_reader.cpp$(DependSuffix) -MM elle/reader.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/elle/reader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/elle_reader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/elle_reader.cpp$(PreprocessSuffix): elle/reader.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/elle_reader.cpp$(PreprocessSuffix) elle/reader.cpp

$(IntermediateDirectory)/ecs_test.cpp$(ObjectSuffix): ecs/test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/ecs_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/ecs_test.cpp$(DependSuffix) -MM ecs/test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/ecs/test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/ecs_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/ecs_test.cpp$(PreprocessSuffix): ecs/test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/ecs_test.cpp$(PreprocessSuffix) ecs/test.cpp

$(IntermediateDirectory)/aeon_aeon.cpp$(ObjectSuffix): aeon/aeon.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/aeon_aeon.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/aeon_aeon.cpp$(DependSuffix) -MM aeon/aeon.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/aeon/aeon.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/aeon_aeon.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/aeon_aeon.cpp$(PreprocessSuffix): aeon/aeon.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/aeon_aeon.cpp$(PreprocessSuffix) aeon/aeon.cpp

$(IntermediateDirectory)/tiny14e_lexer.cpp$(ObjectSuffix): tiny14e/lexer.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/tiny14e_lexer.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/tiny14e_lexer.cpp$(DependSuffix) -MM tiny14e/lexer.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/tiny14e/lexer.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/tiny14e_lexer.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tiny14e_lexer.cpp$(PreprocessSuffix): tiny14e/lexer.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/tiny14e_lexer.cpp$(PreprocessSuffix) tiny14e/lexer.cpp

$(IntermediateDirectory)/dsl_dsl.cpp$(ObjectSuffix): dsl/dsl.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/dsl_dsl.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/dsl_dsl.cpp$(DependSuffix) -MM dsl/dsl.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/dsl/dsl.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/dsl_dsl.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/dsl_dsl.cpp$(PreprocessSuffix): dsl/dsl.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/dsl_dsl.cpp$(PreprocessSuffix) dsl/dsl.cpp

$(IntermediateDirectory)/otto_test.cpp$(ObjectSuffix): otto/test.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/otto_test.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/otto_test.cpp$(DependSuffix) -MM otto/test.cpp
	$(CXX) $(IncludePCH) $(SourceSwitch) "/Users/kenl/wdrive/mygit/rygel/src/main/cpp/otto/test.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/otto_test.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/otto_test.cpp$(PreprocessSuffix): otto/test.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/otto_test.cpp$(PreprocessSuffix) otto/test.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r $(ConfigurationName)/


