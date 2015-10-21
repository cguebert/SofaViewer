#include <core/DocumentFactory.h>

#include <algorithm>
#include <iostream>

static bool documentFactoryCreated = true;

DocumentFactory& DocumentFactory::instance()
{
	static DocumentFactory instance;
	return instance;
}

DocumentFactory::~DocumentFactory()
{
	documentFactoryCreated = false;
}

std::shared_ptr<BaseDocument> DocumentFactory::create(const std::string& name, simplegui::SimpleGUI& gui) const
{
	auto iter = std::find_if(m_documents.begin(), m_documents.end(), [name](const DocumentEntry& entry){
		return entry.name == name;
	});
	if(iter != m_documents.end())
	{
		const auto& creator = iter->creator;
		if(creator)
			return creator->create(gui);
	}

	std::cerr << "Factory has no entry for " << name << std::endl;
	return nullptr;
}

std::shared_ptr<BaseDocument> DocumentFactory::createForFile(const std::string& fileName, simplegui::SimpleGUI& gui) const
{
	auto suffixePos = fileName.find_last_of(".");
	auto suffixe = fileName.substr(suffixePos + 1);
	std::transform(suffixe.begin(), suffixe.end(), suffixe.begin(), ::tolower);
	auto it = std::find_if(m_loadFilesAssociation.begin(), m_loadFilesAssociation.end(),
						   [suffixe](const std::pair<std::string, std::string>& p){
		return p.first == suffixe;
	});

	if(it == m_loadFilesAssociation.end())
		return nullptr;

	return create(it->second, gui);
}

int DocumentFactory::registerDocument(DocumentEntry&& entry)
{
	auto name = entry.name;
	auto iter = std::find_if(m_documents.begin(), m_documents.end(), [name](const DocumentEntry& rhs){
		return rhs.name == name;
	});

	if(iter != m_documents.end())
	{
		std::cerr << "Factory already has an entry for " << entry.name << std::endl;
		return -1;
	}
	else
	{
		int id = m_documents.size();
		m_documents.push_back(std::move(entry));
		recomputeLoadFiles();
		return id;
	}
}

void DocumentFactory::registerModule(const ModuleEntry& entry)
{
	auto iter = std::find_if(m_modules.begin(), m_modules.end(), [entry](const ModuleEntry& rhs){
		return rhs.name == entry.name;
	});
	if(iter != m_modules.end())
		std::cerr << "Factory already has the module " << entry.name << std::endl;
	else
		m_modules.push_back(entry);
}

void DocumentFactory::unregisterModule(const std::string& moduleName)
{
	auto iter = std::find_if(m_modules.begin(), m_modules.end(), [moduleName](const ModuleEntry& rhs){
		return rhs.name == moduleName;
	});
	if(iter == m_modules.end())
	{
		std::cerr << "Factory has no module " << moduleName << std::endl;
		return;
	}


	auto remIt = std::remove_if(m_documents.begin(), m_documents.end(), [moduleName](const DocumentEntry& entry){
		return entry.module == moduleName;
	});
	if (remIt != m_documents.end())
		m_documents.erase(remIt, m_documents.end());
	m_modules.erase(iter);

	recomputeLoadFiles();
}

void DocumentFactory::setModule(int docId, const std::string& module)
{
	m_documents[docId].module = module;
}

std::pair<std::string, std::vector<std::string>> analyseFilter(const std::string& filter)
{
	std::string description;
	std::vector<std::string> suffixes;

	auto npos = std::string::npos;
	auto paren = filter.find('(');
	if(paren != npos)
	{
		auto descEnd = filter.find_last_not_of(" ", paren);
		description = filter.substr(0, descEnd - 1);

		auto end = filter.find_first_not_of("(", paren);

		while(true)
		{
			auto start = filter.find("*.", end);
			if(start == npos) break;
			start += 2;
			end = filter.find_first_of(" )", start);
			if(end == npos) break;

			suffixes.push_back(filter.substr(start, end - start));
		}
	}

	return std::make_pair(description, suffixes);
}

void DocumentFactory::recomputeLoadFiles()
{
	m_loadFilesFilter.clear();
	m_loadFilesAssociation.clear();
	std::vector<std::string> filters, suffixes;

	for(const auto& doc : m_documents)
	{
		for(const auto& filter : doc.loadFiles)
		{
			auto result = analyseFilter(filter);
			if(!result.first.empty() && !result.second.empty())
			{
				filters.push_back(filter);
				for (const auto& suffixe : result.second)
				{
					m_loadFilesAssociation.emplace_back(suffixe, doc.name);
					suffixes.push_back(suffixe);
				}
			}
		}
	}

	if (suffixes.empty())
		return;

	std::sort(filters.begin(), filters.end());
	std::sort(suffixes.begin(), suffixes.end());
	auto last = std::unique(suffixes.begin(), suffixes.end());
	suffixes.erase(last, suffixes.end());

	m_loadFilesFilter = "Supported files (";
	for (const auto& suffixe : suffixes)
		m_loadFilesFilter += "*." + suffixe + " ";
	m_loadFilesFilter.back() = ')'; // Replace the last space

	for(const auto& filter : filters)
		m_loadFilesFilter += ";;" + filter;
}

std::vector<std::string> DocumentFactory::creatableDocuments() const
{
	std::vector<std::string> creatables;
	for (const auto& doc : m_documents)
	{
		if (doc.canCreateNew)
			creatables.push_back(doc.name);
	}

	return creatables;
}

std::string DocumentFactory::loadFilesFilter() const
{
	return m_loadFilesFilter;
}

std::string DocumentFactory::saveFilesFilter(BaseDocument* document) const
{
	if(!document)
		return "";

	auto name = document->documentType();
	auto it = std::find_if(m_documents.begin(), m_documents.end(), [name](const DocumentEntry& entry){
		return entry.name == name;
	});
	if(it == m_documents.end())
		return "";
	const auto doc = *it;

	std::string saveFilesFilter;
	std::vector<std::string> filters;

	for(const auto& filter : doc.saveFiles)
	{
		auto result = analyseFilter(filter);
		if(!result.first.empty())
			filters.push_back(filter);
	}

	std::sort(filters.begin(), filters.end());
	bool first = true;
	for(const auto& filter : filters)
	{
		saveFilesFilter += filter;
		if(!first)
			saveFilesFilter += ";;";
		else
			first = false;
	}

	return saveFilesFilter;
}

//****************************************************************************//

RegisterModule::RegisterModule(const std::string& moduleName)
{
	m_entry.name = moduleName;
}

RegisterModule& RegisterModule::setDescription(const std::string& description)
{
	m_entry.description = description;
	return *this;
}

RegisterModule& RegisterModule::setLicense(const std::string& license)
{
	m_entry.license = license;
	return *this;
}

RegisterModule& RegisterModule::setVersion(const std::string& version)
{
	m_entry.version = version;
	return *this;
}

RegisterModule& RegisterModule::addDocument(int docId)
{
	const auto& docName = DocumentFactory::instance().documents()[docId].name;
	m_entry.documents.push_back(docName);
	DocumentFactory::instance().setModule(docId, m_entry.name);
	return *this;
}

//****************************************************************************//

ModuleHandle::ModuleHandle(const RegisterModule& registerInfo)
	: m_entry(registerInfo.m_entry)
{
	DocumentFactory::instance().registerModule(m_entry);
}

ModuleHandle::~ModuleHandle()
{
	if(documentFactoryCreated)
		DocumentFactory::instance().unregisterModule(m_entry.name);
}
