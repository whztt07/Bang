#include "Bang/BangPreprocessor.h"

#include <algorithm>
#include <iostream>
#include <string>

#include "Bang/Array.h"
#include "Bang/Array.tcc"
#include "Bang/BPReflectedStruct.h"
#include "Bang/File.h"
#include "Bang/List.tcc"
#include "Bang/Path.h"
#include "Bang/SystemProcess.h"

using namespace Bang;

using BP = BangPreprocessor;

const Array<String> BP::Modifiers = {"const",
                                     "constexpr",
                                     "volatile",
                                     "static"};

const Array<String> BP::RVariablePrefixes = {"BANG_REFLECT_VARIABLE"};

const Array<String> BP::RStructPrefixes = {"BANG_REFLECT_CLASS",
                                           "BANG_REFLECT_STRUCT"};

const String BP::ReflectDefinitionsDefineName = "BANG_REFLECT_DEFINITIONS";
const String BP::GetReflectionInfoPtrFuncName = "GetReflectionInfoPtr()";

void BangPreprocessor::Preprocess(const Path &filepath)
{
    File file(filepath);
    String srcContents = file.GetContents();
    String reflHeaderContents;
    bool preprocessedSomething;
    BangPreprocessor::Preprocess(
        srcContents, &reflHeaderContents, &preprocessedSomething);

    String originalExt = filepath.GetExtension();
    Path reflFilepath = filepath.GetDirectory()
                            .Append("." + filepath.GetName())
                            .AppendExtension("refl")
                            .AppendExtension(originalExt);

    bool writePreprocessedFile = true;
    if (reflFilepath.Exists())
    {
        String oldReflContents = File(reflFilepath).GetContents();
        writePreprocessedFile = (oldReflContents != reflHeaderContents);
    }

    if (writePreprocessedFile)
    {
        File::Write(reflFilepath, reflHeaderContents);

        if (preprocessedSomething)
        {
            std::cout << "  File '" << filepath.GetAbsolute().ToCString()
                      << "' successfully preprocessed into '"
                      << reflFilepath.GetAbsolute().ToCString() << "'"
                      << std::endl;
        }
    }
}

void BangPreprocessor::Preprocess(const String &source,
                                  String *_reflectionHeaderSource,
                                  bool *preprocessedSomething)
{
    *preprocessedSomething = false;
    String &reflectionHeaderSource = *_reflectionHeaderSource;
    reflectionHeaderSource = R"VERBATIM(
             #include "Bang/BPReflectedStruct.h"
             #include "Bang/BPReflectedVariable.h"
             #include "Bang/MetaNode.h"

          )VERBATIM";

    Array<BPReflectedStruct> reflectStructs =
        BangPreprocessor::GetReflectStructs(source);
    for (const BPReflectedStruct &reflStruct : reflectStructs)
    {
        String reflectDefineCode =
            "#define  REFLECT_DEFINITIONS_DEFINE_NAME_RSTRUCT_VAR_NAME() ";
        reflectDefineCode += R"VERBATIM( public:
                    GET_REFLECTION_INFO_CODE
                    GET_READ_REFLECTION_CODE
                    GET_WRITE_REFLECTION_CODE
                private:   )VERBATIM";
        reflectDefineCode.ReplaceInSitu("GET_REFLECTION_INFO_CODE",
                                        reflStruct.GetGetReflectionInfoCode());
        reflectDefineCode.ReplaceInSitu("GET_READ_REFLECTION_CODE",
                                        reflStruct.GetReadReflectionCode());
        reflectDefineCode.ReplaceInSitu("GET_WRITE_REFLECTION_CODE",
                                        reflStruct.GetWriteReflectionCode());
        reflectDefineCode.ReplaceInSitu("\n", "\\\n");

        reflectDefineCode +=
            "\n"
            "#undef REFLECT_DEFINITIONS_DEFINE_NAME \n"
            "#define REFLECT_DEFINITIONS_DEFINE_NAME(ClassName) \
                         REFLECT_DEFINITIONS_DEFINE_NAME_##ClassName()";

        reflectDefineCode.ReplaceInSitu("RSTRUCT_VAR_NAME",
                                        reflStruct.GetStructVariableName());
        reflectDefineCode.ReplaceInSitu("REFLECT_DEFINITIONS_DEFINE_NAME",
                                        BP::ReflectDefinitionsDefineName);

        reflectionHeaderSource += reflectDefineCode;
        *preprocessedSomething = true;
    }
}

Array<BPReflectedStruct> BangPreprocessor::GetReflectStructs(
    const Path &sourcePath)
{
    String source = File::GetContents(sourcePath);
    return BangPreprocessor::GetReflectStructs(source);
}

Array<BPReflectedStruct> BangPreprocessor::GetReflectStructs(
    const String &source)
{
    Array<BPReflectedStruct> reflectStructsArray;

    String src = source;
    BP::RemoveComments(&src);
    String::Iterator it = src.Begin();
    while (true)
    {
        // Find Structure/Class annotation
        String::Iterator itStructBegin =
            BP::Find(it, src.End(), BP::RStructPrefixes);
        if (itStructBegin == src.End())
        {
            break;
        }

        String::Iterator itStructScopeBegin, itStructScopeEnd;
        BP::GetNextScope(itStructBegin,
                         src.End(),
                         &itStructScopeBegin,
                         &itStructScopeEnd,
                         '{',
                         '}');
        it = itStructScopeEnd;
        if (itStructScopeBegin == src.End())
        {
            break;
        }

        bool ok;
        BPReflectedStruct reflStruct;
        BPReflectedStruct::FromString(
            itStructBegin, itStructScopeEnd, &reflStruct, &ok);
        reflectStructsArray.PushBack(reflStruct);
    }

    return reflectStructsArray;
}

void BangPreprocessor::RemoveComments(String *source)
{
    String &src = *source;

    SystemProcess gCompilerProcess;
    gCompilerProcess.Start("g++", {"-fpreprocessed", "-E", "-"});

    gCompilerProcess.Write(src);
    gCompilerProcess.CloseWriteChannel();
    gCompilerProcess.WaitUntilFinished();

    String output = gCompilerProcess.ReadStandardOutput();
    gCompilerProcess.Close();

    if (output.Size() >= 1)
    {
        output.Remove(output.Begin(),
                      output.Find('\n') + 2);  // Remove first line
    }
    *source = output;
}

String::Iterator BangPreprocessor::Find(String::Iterator begin,
                                        String::Iterator end,
                                        const Array<String> &toFindList)
{
    for (const String &toFind : toFindList)
    {
        String::Iterator itFound =
            std::search(begin, end, toFind.Begin(), toFind.End());
        if (itFound != end)
        {
            return itFound;
        }
    }
    return end;
}

void BangPreprocessor::GetNextScope(String::Iterator begin,
                                    String::Iterator end,
                                    String::Iterator *scopeBegin,
                                    String::Iterator *scopeEnd,
                                    char openingBrace,
                                    char closingBrace)
{
    int insideness = 0;
    String::Iterator it = begin;
    for (; it != end; ++it)
    {
        if (*it == openingBrace)
        {
            if (insideness == 0)
            {
                *scopeBegin = it;
            }
            ++insideness;
        }
        else if (*it == closingBrace)
        {
            --insideness;
            if (insideness == 0)
            {
                *scopeEnd = it + 1;
                break;
            }
        }
    }

    if (insideness != 0)
    {
        *scopeBegin = *scopeEnd = end;
    }
}

void BangPreprocessor::SkipBlanks(String::Iterator *it, String::Iterator end)
{
    char c = *(*it);
    while (c == '\n' || c == '\r' || c == '\t' || c == ' ')
    {
        ++(*it);
        if (*it == end)
        {
            break;
        }
        c = *(*it);
    }
}

void BangPreprocessor::SkipUntilNextBlank(String::Iterator *it,
                                          String::Iterator end)
{
    SkipUntilNext(it, end, {"\n", "\r", "\t", " "});
}

void BangPreprocessor::SkipUntilNext(String::Iterator *it,
                                     String::Iterator end,
                                     const Array<String> &particles)
{
    String c(std::string(1, *(*it)));
    while (!particles.Contains(c))
    {
        ++(*it);
        if (*it == end)
        {
            break;
        }
        c = String(std::string(1, *(*it)));
    }
}

void BangPreprocessor::FindNextWord(String::Iterator begin,
                                    String::Iterator end,
                                    String::Iterator *wordBegin,
                                    String::Iterator *wordEnd)
{
    *wordBegin = begin;
    BP::SkipBlanks(wordBegin, end);

    *wordEnd = (*wordBegin + 1);
    char c = *(*wordEnd);
    while (String::IsLetter(c) || String::IsNumber(c) || c == '_')
    {
        ++(*wordEnd);
        if (*wordEnd == end)
        {
            break;
        }
        c = *(*wordEnd);
    }
}
