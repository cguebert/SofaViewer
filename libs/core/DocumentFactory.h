#pragma once

#include <core/BaseDocument.h>

#include <vector>

class BaseDocumentCreator
{
public:
	virtual ~BaseDocumentCreator() {}
	virtual std::shared_ptr<BaseDocument> create(const std::string& type) const = 0;
};

class CORE_API DocumentFactory
{
public:
	static DocumentFactory& instance();
	~DocumentFactory();

	std::shared_ptr<BaseDocument> create(const std::string& name) const;

	std::vector<std::string> documentsToLoadFile(const std::string& fileName) const;
	std::vector<std::string> creatableDocuments() const;

	std::string loadFilesFilter() const; // For all documents
	std::string saveFilesFilter(const BaseDocument* document) const; // Only for current document

	struct DocumentEntry
	{
		bool canCreateNew = false;
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
		std::string dirPath;
		std::vector<std::string> documents;
	};

	typedef std::vector<DocumentEntry> DocumentsList;
	const DocumentsList& documents() const
	{ return m_documents; }

	const DocumentEntry& document(const std::string& name) const;

	typedef std::vector<ModuleEntry> ModulesList;
	const ModulesList& modules() const
	{ return m_modules; }

	const ModuleEntry& module(const std::string& name) const;

	void setModuleDirPath(const std::string& path)
	{ m_currentModuleDirPath = path; }

protected:
	template<class T> friend class RegisterDocument;
	int registerDocument(DocumentEntry&& entry);
	void recomputeLoadFiles();

	friend class ModuleHandle;
	void registerModule(ModuleEntry entry);
	void unregisterModule(const std::string& module);

	friend class RegisterModule;
	void setModule(int docId, const std::string& module);

	DocumentsList m_documents;
	ModulesList m_modules;
	std::string m_currentModuleDirPath; // For the module we are currently adding
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
	std::shared_ptr<BaseDocument> create(const std::string& type) const override
	{ return std::make_shared<T>(type); }
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

	RegisterDocument& canCreateNew(bool createNew)
	{ m_entry.canCreateNew = createNew; return *this; }

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
