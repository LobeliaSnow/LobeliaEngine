#include "Lobelia.hpp"
#include "XML.hpp"

namespace Lobelia {
	ComPtr<IXmlReader> XMLSystem::reader;
	ComPtr<IXmlWriter> XMLSystem::writer;

	void XMLSystem::Initialize() {
		HRESULT hr = S_OK;
		hr = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(reader.GetAddressOf()), 0);
		if (FAILED(hr))STRICT_THROW("XML Reader�쐬�Ɏ��s");
		hr = CreateXmlWriter(__uuidof(IXmlWriter), reinterpret_cast<void**>(XMLSystem::writer.GetAddressOf()), 0);
		if (FAILED(hr))STRICT_THROW("");
	}
	XMLParser::XMLParser(const char* file_path) {
		HRESULT hr = S_OK;
		//�t�@�C���X�g���[���쐬
		hr = SHCreateStreamOnFile(file_path, STGM_READ, stream.GetAddressOf());
		if (FAILED(hr))STRICT_THROW("�X�g���[���̍쐬�Ɏ��s���܂���");
		hr = XMLSystem::reader->SetInput(stream.Get());
		if (FAILED(hr))STRICT_THROW("���͐ݒ莸�s");
		//�m�[�h�����ăf�[�^�̒T��
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
			case XmlNodeType_Element://�v�f�̊J�n
				hr = XMLSystem::reader->GetLocalName(&localName, nullptr);
				(*now.rbegin())->data = Utility::ConverteString(localName);
				now.push_back(&(*now.rbegin())->other[Utility::ConverteString(localName)]);
				hierarchy++;
				break;
			case XmlNodeType_EndElement:
				now.erase(std::next(now.begin(), hierarchy));
				hierarchy--;
				break;
			case XmlNodeType_Text://�e�L�X�g
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
		if (FAILED(hr))STRICT_THROW("�X�g���[���̍쐬�Ɏ��s���܂���");
		hr = XMLSystem::writer->SetOutput(stream.Get());
		if (FAILED(hr))STRICT_THROW("�o�͐ݒ莸�s");
		hr = XMLSystem::writer->SetProperty(XmlWriterProperty_Indent, true);
		if (FAILED(hr))STRICT_THROW("�v���p�e�B�̐ݒ�Ɏ��s");
		hr = XMLSystem::writer->WriteStartDocument(XmlStandalone_Omit);
		if (FAILED(hr))STRICT_THROW("�h�L�������g�f�[�^�쐬");
	}
	XMLWriter::~XMLWriter() {
		XMLSystem::writer->WriteEndDocument();
		XMLSystem::writer->Flush();
	}
	void XMLWriter::StartElement(const char* element) {
		HRESULT hr = XMLSystem::writer->WriteStartElement(nullptr, Utility::ConverteWString(element).c_str(), nullptr);
		if (FAILED(hr))STRICT_THROW("�v�f�̏������݂Ɏ��s");
	}
	void XMLWriter::EndElement() {
		HRESULT hr = XMLSystem::writer->WriteFullEndElement();
		if (FAILED(hr))STRICT_THROW("�v�f�̏������݂Ɏ��s");
	}
	void XMLWriter::Write(const char* tag, const char* data) {
		HRESULT hr = S_OK;
		hr = XMLSystem::writer->WriteElementString(nullptr, Utility::ConverteWString(tag).c_str(), nullptr, Utility::ConverteWString(data).c_str());
		if (FAILED(hr))STRICT_THROW("�f�[�^�̏������݂Ɏ��s");
	}
}