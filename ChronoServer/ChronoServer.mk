##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=ChronoServer
ConfigurationName      :=Debug
WorkspacePath          := "/home/dylan/Documents/BoostNetworking"
ProjectPath            := "/home/dylan/Documents/BoostNetworking/ChronoServer"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=
Date                   :=08/01/16
CodeLitePath           :="/home/dylan/.codelite"
LinkerName             :=g++
SharedObjectLinkerName :=g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-gstab
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
ObjectsFileList        :="ChronoServer.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -lpthread
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)/home/dylan/boost_1_61_0/include $(IncludeSwitch)/home/dylan/Documents/BoostNetworking/Vehicle_Protobuf_Messages 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)boost_system $(LibrarySwitch)protobuf 
ArLibs                 :=  "boost_system" "protobuf" 
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := g++
CC       := gcc
CXXFLAGS :=  -g -O0 -Wall -std=c++11 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IntermediateDirectory)/World.cpp$(ObjectSuffix) $(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(ObjectSuffix) 



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
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/main.cpp$(ObjectSuffix): main.cpp $(IntermediateDirectory)/main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/dylan/Documents/BoostNetworking/ChronoServer/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/main.cpp$(DependSuffix): main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/main.cpp$(DependSuffix) -MM "main.cpp"

$(IntermediateDirectory)/main.cpp$(PreprocessSuffix): main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/main.cpp$(PreprocessSuffix) "main.cpp"

$(IntermediateDirectory)/World.cpp$(ObjectSuffix): World.cpp $(IntermediateDirectory)/World.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/dylan/Documents/BoostNetworking/ChronoServer/World.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/World.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/World.cpp$(DependSuffix): World.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/World.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/World.cpp$(DependSuffix) -MM "World.cpp"

$(IntermediateDirectory)/World.cpp$(PreprocessSuffix): World.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/World.cpp$(PreprocessSuffix) "World.cpp"

$(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(ObjectSuffix): ../Vehicle_Protobuf_Messages/ChronoMessages.pb.cc $(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/dylan/Documents/BoostNetworking/Vehicle_Protobuf_Messages/ChronoMessages.pb.cc" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(DependSuffix): ../Vehicle_Protobuf_Messages/ChronoMessages.pb.cc
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(ObjectSuffix) -MF$(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(DependSuffix) -MM "../Vehicle_Protobuf_Messages/ChronoMessages.pb.cc"

$(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(PreprocessSuffix): ../Vehicle_Protobuf_Messages/ChronoMessages.pb.cc
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/Vehicle_Protobuf_Messages_ChronoMessages.pb.cc$(PreprocessSuffix) "../Vehicle_Protobuf_Messages/ChronoMessages.pb.cc"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


