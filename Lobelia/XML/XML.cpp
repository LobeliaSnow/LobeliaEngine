#include "Lobelia.hpp"
#include "XML.hpp"

namespace Lobelia {
	ComPtr<IXmlReader> XMLSystem::reader;
	ComPtr<IXmlWriter> XMLSystem::writer;

	void XMLSystem::Initialize() {
		HRESULT hr = S_OK;
		hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(reader.GetAddressOf()), 0);
		if (FAILED(hr))STRICT_THROW("XML Reader作成に失敗");
		hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(XMLSystem::writer.GetAddressOf()), 0);
		if (FAILED(hr))STRICT_THROW("");
	}
	XMLParser::XMLParser(const char* file_path) {
		HRESULT hr = S_OK;
		//ファイルストリーム作成
		hr = SHCreateStreamOnFile(file_path, STGM_READ, stream.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("ストリームの作成に失敗しました");
		hr = XMLSystem::reader->SetInput(stream.Get());
		if (FAILED(hr))STRICT_THROW("入力設定失敗");
		//ノードを見てデータの探索
		SearchNode();
	}
	void XMLParser::SearchNode() {
		XmlNodeType nodeType;
		LPCWSTR localName;
		LPCWSTR value;
		HRESULT hr = S_OK;
		int hierarchy = 0;
		std::list<Tree*> now;
		now.push_back(&root);
		while (XMLSystem::reader->Read(&nodeType) == S_OK) {
			switch (nodeType) {
			case XmlNodeType_Element://要素の開始
				hr = XMLSystem::reader->GetLocalName(&localName, nullptr);
				(*now.rbegin())->data = Utility::ConverteString(localName);
				now.push_back(&(*now.rbegin())->other[Utility::ConverteString(localName)]);
				hierarchy++;
				break;
			case XmlNodeType_EndElement:
				now.erase(std::next(now.begin(), hierarchy));
				hierarchy--;
				break;
			case XmlNodeType_Text://テキスト
				hr = XMLSystem::reader->GetValue(&value, nullptr);
				(*now.rbegin())->data = Utility::ConverteString(value);
				break;
			default:	break;
			}
		}
	}

	XMLWriter::XMLWriter(const char* file_path) {
		HRESULT hr = S_OK;
		hr = SHCreateStreamOnFile(file_path, STGM_CREATE | STGM_WRITE, stream.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("ストリームの作成に失敗しました");
		hr = XMLSystem::writer->SetOutput(stream.Get());
		if (FAILED(hr))STRICT_THROW("出力設定失敗");
		hr = XMLSystem::writer->SetProperty(XmlWriterProperty_Indent, true);
		if (FAILED(hr))STRICT_THROW("プロパティの設定に失敗");
		hr = XMLSystem::writer->WriteStartDocument(XmlStandalone_Omit);
		if (FAILED(hr))STRICT_THROW("ドキュメントデータ作成");
	}
	XMLWriter::~XMLWriter() {
		XMLSystem::writer->WriteEndDocument();
		XMLSystem::writer->Flush();
	}
	void XMLWriter::StartElement(const char* element) {
		HRESULT hr = XMLSystem::writer->WriteStartElement(nullptr, Utility::ConverteWString(element).c_str(), nullptr);
		if (FAILED(hr))STRICT_THROW("要素の書き込みに失敗");
	}
	void XMLWriter::EndElement() {
		HRESULT hr = XMLSystem::writer->WriteFullEndElement();
		if (FAILED(hr))STRICT_THROW("要素の書き込みに失敗");
	}
	void XMLWriter::Write(const char* tag, const char* data) {
		HRESULT hr = S_OK;
		hr = XMLSystem::writer->WriteElementString(nullptr, Utility::ConverteWString(tag).c_str(), nullptr, Utility::ConverteWString(data).c_str());
		if (FAILED(hr))STRICT_THROW("データの書き込みに失敗");
	}
}