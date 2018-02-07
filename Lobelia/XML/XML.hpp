#pragma once
#include <xmllite.h>
#pragma comment(lib, "xmllite.lib")

namespace Lobelia {
	class XMLSystem {
		friend class XMLParser;
		friend class XMLWriter;
	private:
		static ComPtr<IXmlReader> reader;
		static ComPtr<IXmlWriter> writer;
	public:
		static void Initialize();
	};
	class XMLParser {
	public:
		class Tree {
			friend class XMLParser;
		private:
			std::map<std::string, Tree> other;
			std::string data;
		public:
			Tree & Next(std::string key) {
				if (other.find(key) == other.end())STRICT_THROW("not exist hierarchy");
				return other[key];
			}
			Tree& operator[](std::string key) { return Next(key); }
			const std::string& GetValue() { return data; }
			double GetDouble()const { return std::stod(data); }
			float GetFloat()const { return std::stof(data); }
			int GetInt()const { return std::stoi(data); }
		};
	private:
		ComPtr<IStream> stream;
		Tree root;
	private:
		void SearchNode();
	public:
		XMLParser(const char* file_path);
		~XMLParser() = default;
		Tree& Root() { return root.other.begin()->second; }
	};
	class XMLWriter {
	private:
		ComPtr<IStream> stream;
	public:
		XMLWriter(const char* file_path);
		~XMLWriter();
		void StartElement(const char* element);
		void EndElement();
		void Write(const char* tag, const char* data);
	};

}