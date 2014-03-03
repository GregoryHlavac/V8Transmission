// Oil Change.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <v8.h>
#include <assert.h>
#include <fcntl.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "V8Transmission.h"

using namespace v8;

using namespace V8Transmission;
using namespace V8Transmission::TypeConversion;
using namespace V8Transmission::TypeConversion::Internal;


Handle<v8::Context> CreateShellContext(v8::Isolate* isolate);
void RunShell(Handle<v8::Context> context);
int RunMain(v8::Isolate* isolate, int argc, char* argv[]);
bool ExecuteString(v8::Isolate* isolate,
	Handle<String> source,
	Handle<Value> name,
	bool print_result,
	bool report_exceptions);
void Print(const v8::FunctionCallbackInfo<Value>& args);
void Read(const v8::FunctionCallbackInfo<Value>& args);
void Load(const v8::FunctionCallbackInfo<Value>& args);
void Quit(const v8::FunctionCallbackInfo<Value>& args);
void Version(const v8::FunctionCallbackInfo<Value>& args);
Handle<String> ReadFile(v8::Isolate* isolate, const char* name);
void ReportException(v8::Isolate* isolate, v8::TryCatch* handler);


void BindDouble(const v8::FunctionCallbackInfo<Value>& args)
{
	args.GetReturnValue().Set(ConvertToJS(args.GetIsolate(), 20.0f));
}


static bool run_shell;

int main(int argc, char* argv[]) 
{
	v8::V8::InitializeICU();
	v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	run_shell = (argc == 1);
	int result;
	{
		HandleScope handle_scope(isolate);
		Handle<v8::Context> context = CreateShellContext(isolate);
		if (context.IsEmpty()) {
			fprintf(stderr, "Error creating context\n");
			return 1;
		}
		context->Enter();

		Handle<Value> stval = ConvertToJS(isolate, std::string("Hurr, Durr"));

		result = RunMain(isolate, argc, argv);
		if (run_shell) RunShell(context);

		context->Exit();
	}
	v8::V8::Dispose();
	return result;
}


// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}


int xc(std::string first, std::string second)
{
	std::cout << "First & Second" << std::endl << first << "|" << second << std::endl;

	return 0;
}

int xcx(std::string first, std::string second)
{
	std::cout << "FirstX & SecondX" << std::endl << first << "|" << second << std::endl;

	return 0;
}


class RandomCrap
{
	int XPrint()
	{
		std::cout << "RandomCrap XPrint Called" << std::endl;
	}
};

namespace V8Transmission
{
	namespace TypeConversion
	{
		template<>
		struct ShiftNative<RandomCrap*>
		{
			RandomCrap* operator()(v8::Isolate* iso, const v8::Handle<v8::Value>& val) const
			{
				return new RandomCrap;
			}
		};
	}
}


// Creates a new execution environment containing the built-in
// functions.
Handle<v8::Context> CreateShellContext(v8::Isolate* isolate) {
	// Create a template for the global object.
	Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

	global->Set(String::NewFromUtf8(isolate, "print"), FunctionTemplate::New(isolate, Print));
	global->Set(String::NewFromUtf8(isolate, "read"), FunctionTemplate::New(isolate, Read));
	global->Set(String::NewFromUtf8(isolate, "load"), FunctionTemplate::New(isolate, Load));
	global->Set(String::NewFromUtf8(isolate, "quit"), FunctionTemplate::New(isolate, Quit));
	global->Set(String::NewFromUtf8(isolate, "version"), FunctionTemplate::New(isolate, Version));

	global->Set(String::NewFromUtf8(isolate, "dblValue"), FunctionTemplate::New(isolate, BindDouble));

	global->Set(String::NewFromUtf8(isolate, "gear"), FunctionTemplate::New(isolate, StaticFunctionGear<int, std::string, std::string>::Invoke<xc>));
	global->Set(String::NewFromUtf8(isolate, "gearx"), FunctionTemplate::New(isolate, StaticFunctionGear<int, std::string, std::string>::Invoke<xcx>));


	return v8::Context::New(isolate, NULL, global);
}


// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo<Value>& args) {
	bool first = true;

	for (int i = 0; i < args.Length(); i++) {
		HandleScope handle_scope(args.GetIsolate());
		if (first) {
			first = false;
		}
		else {
			printf(" ");
		}
		String::Utf8Value str(args[i]);
		const char* cstr = ToCString(str);
		printf("%s", cstr);
	}
	printf("\n");
	fflush(stdout);
}


// The callback that is invoked by v8 whenever the JavaScript 'read'
// function is called.  This function loads the content of the file named in
// the argument into a JavaScript string.
void Read(const v8::FunctionCallbackInfo<Value>& args) {
	if (args.Length() != 1) {
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Bad parameters"));
		return;
	}
	String::Utf8Value file(args[0]);
	if (*file == NULL) {
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
		return;
	}
	Handle<String> source = ReadFile(args.GetIsolate(), *file);
	if (source.IsEmpty()) {
		args.GetIsolate()->ThrowException(
			String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
		return;
	}
	args.GetReturnValue().Set(source);
}


// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
void Load(const v8::FunctionCallbackInfo<Value>& args) {
	for (int i = 0; i < args.Length(); i++) {
		HandleScope handle_scope(args.GetIsolate());
		String::Utf8Value file(args[i]);
		if (*file == NULL) {
			args.GetIsolate()->ThrowException(
				String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
			return;
		}
		Handle<String> source = ReadFile(args.GetIsolate(), *file);
		if (source.IsEmpty()) {
			args.GetIsolate()->ThrowException(
				String::NewFromUtf8(args.GetIsolate(), "Error loading file"));
			return;
		}
		if (!ExecuteString(args.GetIsolate(),
			source,
			String::NewFromUtf8(args.GetIsolate(), *file),
			false,
			false)) {
			args.GetIsolate()->ThrowException(
				String::NewFromUtf8(args.GetIsolate(), "Error executing file"));
			return;
		}
	}
}


// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.  Quits.
void Quit(const v8::FunctionCallbackInfo<Value>& args) {
	// If not arguments are given args[0] will yield undefined which
	// converts to the integer value 0.
	int exit_code = args[0]->Int32Value();
	fflush(stdout);
	fflush(stderr);
	exit(exit_code);
}


void Version(const v8::FunctionCallbackInfo<Value>& args) {
	args.GetReturnValue().Set(
		String::NewFromUtf8(args.GetIsolate(), v8::V8::GetVersion()));
}


// Reads a file into a v8 string.
Handle<String> ReadFile(v8::Isolate* isolate, const char* name) {
	FILE* file = fopen(name, "rb");
	if (file == NULL) return Handle<String>();

	fseek(file, 0, SEEK_END);
	int size = ftell(file);
	rewind(file);

	char* chars = new char[size + 1];
	chars[size] = '\0';
	for (int i = 0; i < size;) {
		int read = static_cast<int>(fread(&chars[i], 1, size - i, file));
		i += read;
	}
	fclose(file);
	Handle<String> result =
		String::NewFromUtf8(isolate, chars, String::kNormalString, size);
	delete[] chars;
	return result;
}


// Process remaining command line arguments and execute files
int RunMain(v8::Isolate* isolate, int argc, char* argv[]) {
	for (int i = 1; i < argc; i++) {
		const char* str = argv[i];
		if (strcmp(str, "--shell") == 0) {
			run_shell = true;
		}
		else if (strcmp(str, "-f") == 0) {
			// Ignore any -f flags for compatibility with the other stand-
			// alone JavaScript engines.
			continue;
		}
		else if (strncmp(str, "--", 2) == 0) {
			fprintf(stderr,
				"Warning: unknown flag %s.\nTry --help for options\n", str);
		}
		else if (strcmp(str, "-e") == 0 && i + 1 < argc) {
			// Execute argument given to -e option directly.
			Handle<String> file_name =
				String::NewFromUtf8(isolate, "unnamed");
			Handle<String> source =
				String::NewFromUtf8(isolate, argv[++i]);
			if (!ExecuteString(isolate, source, file_name, false, true)) return 1;
		}
		else {
			// Use all other arguments as names of files to load and run.
			Handle<String> file_name = String::NewFromUtf8(isolate, str);
			Handle<String> source = ReadFile(isolate, str);
			if (source.IsEmpty()) {
				fprintf(stderr, "Error reading '%s'\n", str);
				continue;
			}
			if (!ExecuteString(isolate, source, file_name, false, true)) return 1;
		}
	}
	return 0;
}


// The read-eval-execute loop of the shell.
void RunShell(Handle<v8::Context> context) {
	fprintf(stderr, "V8 version %s [sample shell]\n", v8::V8::GetVersion());
	static const int kBufferSize = 256;
	// Enter the execution environment before evaluating any code.
	v8::Context::Scope context_scope(context);
	v8::Local<String> name(
		String::NewFromUtf8(context->GetIsolate(), "(shell)"));
	while (true) {
		char buffer[kBufferSize];
		fprintf(stderr, "> ");
		char* str = fgets(buffer, kBufferSize, stdin);
		if (str == NULL) break;
		HandleScope handle_scope(context->GetIsolate());
		ExecuteString(context->GetIsolate(),
			String::NewFromUtf8(context->GetIsolate(), str),
			name,
			true,
			true);
	}
	fprintf(stderr, "\n");
}


// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate,
	Handle<String> source,
	Handle<Value> name,
	bool print_result,
	bool report_exceptions) {
	HandleScope handle_scope(isolate);
	v8::TryCatch try_catch;
	Handle<v8::Script> script = v8::Script::Compile(source, name);
	if (script.IsEmpty()) {
		// Print errors that happened during compilation.
		if (report_exceptions)
			ReportException(isolate, &try_catch);
		return false;
	}
	else {
		Handle<Value> result = script->Run();
		if (result.IsEmpty()) {
			assert(try_catch.HasCaught());
			// Print errors that happened during execution.
			if (report_exceptions)
				ReportException(isolate, &try_catch);
			return false;
		}
		else {
			assert(!try_catch.HasCaught());
			if (print_result && !result->IsUndefined()) {
				// If all went well and the result wasn't undefined then print
				// the returned value.
				String::Utf8Value str(result);
				const char* cstr = ToCString(str);
				printf("%s\n", cstr);
			}
			return true;
		}
	}
}


void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
	HandleScope handle_scope(isolate);
	String::Utf8Value exception(try_catch->Exception());
	const char* exception_string = ToCString(exception);
	Handle<v8::Message> message = try_catch->Message();
	if (message.IsEmpty()) {
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		fprintf(stderr, "%s\n", exception_string);
	}
	else {
		// Print (filename):(line number): (message).
		String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber();
		fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
		// Print line of source code.
		String::Utf8Value sourceline(message->GetSourceLine());
		const char* sourceline_string = ToCString(sourceline);
		fprintf(stderr, "%s\n", sourceline_string);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) {
			fprintf(stderr, " ");
		}
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) {
			fprintf(stderr, "^");
		}
		fprintf(stderr, "\n");
		String::Utf8Value stack_trace(try_catch->StackTrace());
		if (stack_trace.length() > 0) {
			const char* stack_trace_string = ToCString(stack_trace);
			fprintf(stderr, "%s\n", stack_trace_string);
		}
	}
}