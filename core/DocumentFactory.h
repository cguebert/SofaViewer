#pragma once

#include <core/BaseDocument.h>

#include <map>
#include <vector>

class BaseDocumentCreator
{
public:
	virtual ~BaseDocumentCreator() {}
	virtual std::shared_ptr<BaseDocument> create(ui::SimpleGUI& /*gui*/) const = 0;
};

class CORE_API DocumentFactory
{
public:
	static DocumentFactory& instance();
	~DocumentFactory();

	std::shared_ptr<BaseDocument> create(const std::string& name, ui::SimpleGUI& gui) const;
	std::shared_ptr<BaseDocument> createForFile(const std::string& fileName, ui::SimpleGUI& gui) const;

	std::string loadFilesFilter() const; // For all documents
	std::string saveFilesFilter(BaseDocument* document) const; // Only for current document

	struct DocumentEntry
	{
		std::string name;
		std::string description;
		std::string module;

		std::vector<std::string> loadFiles, saveFiles;
		std::shared_ptr<BaseDocumentCreator> creator;
	};

	struct ModuleEntry
	{
		std::string name;
		std::string description;
		std::string license;
		std::string version;
		std::vector<std::string> documents;
	};

	typedef std::vector<DocumentEntry> DocumentsList;
	const DocumentsList& documents() const
	{ return m_documents; }

	typedef std::vector<ModuleEntry> ModulesList;
	const ModulesList& modules() const
	{ return m_modules; }

protected:
	template<class T> friend class RegisterDocument;
	int registerDocument(DocumentEntry&& entry);
	void recomputeLoadFiles();

	friend class ModuleHandle;
	void registerModule(const ModuleEntry& entry);
	void unregisterModule(const std::string& module);

	friend class RegisterModule;
	void setModule(int docId, const std::string& module);

	DocumentsList m_documents;
	ModulesList m_modules;
	std::string m_loadFilesFilter;
	std::vector<std::pair<std::string, std::string>> m_loadFilesAssociation;

private:
	DocumentFactory() {}
};

//****************************************************************************//

template <class T>
class DocumentCreator : public BaseDocumentCreator
{
public:
	std::shared_ptr<BaseDocument> create(ui::SimpleGUI& gui) const override
	{ return std::make_shared<T>(gui); }
};

template <class T>
class RegisterDocument
{
public:
	RegisterDocument() = delete;
	explicit RegisterDocument(const std::string& uniqueName)
	{
		m_entry.name = uniqueName;
		m_entry.creator = std::make_shared<DocumentCreator<T>>();
	}

	RegisterDocument& setDescription(const std::string& description)
	{ m_entry.description = description; return *this; }

	RegisterDocument& addLoadFile(const std::string& filter)
	{ m_entry.loadFiles.push_back(filter); return *this; }

	RegisterDocument& addSaveFile(const std::string& filter)
	{ m_entry.saveFiles.push_back(filter); return *this; }

	operator int()
	{ return DocumentFactory::instance().registerDocument(std::move(m_entry)); }

protected:
	DocumentFactory::DocumentEntry m_entry;
};


//****************************************************************************//

class CORE_API RegisterModule
{
public:
	RegisterModule() = delete;
	explicit RegisterModule(const std::string& moduleName);
	RegisterModule& setDescription(const std::string& description);
	RegisterModule& setLicense(const std::string& license);
	RegisterModule& setVersion(const std::string& version);
	RegisterModule& addDocument(int documentId); // The result of RegisterDocument

protected:
	DocumentFactory::ModuleEntry m_entry;

	friend class ModuleHandle;
};

// This class is used by RegisterModule to automatically unregister a module when unloading a library
class CORE_API ModuleHandle
{
public:
	ModuleHandle() = delete;
	ModuleHandle(const RegisterModule& registerInfo);
	~ModuleHandle();

	DocumentFactory::ModuleEntry m_entry;
};
