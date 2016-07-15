#include "stdafx.h"
#include "string.h"
#include "System.h"
#include <fstream>
#include <Constants.h>

using namespace std;

CPPConsole::System::System() {
    _system = nullptr;
    _dataSourceList = nullptr;
}

bool CPPConsole::System::InitializeSdk() {
    VxSdk::VxResult::Value result = VxSdk::VxInit(Constants::kSdkKey);
    if (result == VxSdk::VxResult::kOK) {
        return true;
    }
    return false;
}

bool CPPConsole::System::Login(const char* ip, int port, const char* user, const char* passwd) {
    VxSdk::VxLoginInfo loginInfo;
    loginInfo.port = Constants::kPortnum;
    loginInfo.useSsl = true;
    VxSdk::Utilities::StrCopySafe(loginInfo.ipAddress, ip);
    VxSdk::Utilities::StrCopySafe(loginInfo.username, user);
    VxSdk::Utilities::StrCopySafe(loginInfo.password, passwd);

    VxSdk::IVxSystem* system = nullptr;
    VxSystemLogin(loginInfo, system);
    _system = system;
    return _system != nullptr;
}

list<CPPConsole::DataSource*>* CPPConsole::System::GetDataSources() {
    if (_dataSourceList != nullptr) {
        for (list<DataSource*>::const_iterator iterator = _dataSourceList->begin(), end = _dataSourceList->end(); iterator != end; ++iterator) {
            delete *iterator;
        }
        delete _dataSourceList;
    }

    _dataSourceList = new list<DataSource*>();
    VxSdk::VxCollection<VxSdk::IVxDataSource**> dataSources;
    VxSdk::VxResult::Value result = _system->GetDataSources(dataSources);
    if (result == VxSdk::VxResult::kInsufficientSize) {
        dataSources.collection = new VxSdk::IVxDataSource*[dataSources.collectionSize];
        result = _system->GetDataSources(dataSources);
        if (result == VxSdk::VxResult::kOK) {
            for (int i = 0; i < dataSources.collectionSize; i++)
                _dataSourceList->push_back(new DataSource(dataSources.collection[i]));
        }
        delete[] dataSources.collection;
    }
    return _dataSourceList;
}

list<CPPConsole::DataSource*>* CPPConsole::System::GetDataSources(int pageIndex, int count, int& totalItems) {
    list<DataSource*>* dataSourceList = new list<DataSource*>();

    VxSdk::VxCollection<VxSdk::IVxDataSource**> dataSources;

    dataSources.filterSize = 2;
    VxSdk::VxCollectionFilter filters[2];
    filters[0].key = VxSdk::VxCollectionFilterItem::kStart;
    filters[1].key = VxSdk::VxCollectionFilterItem::kCount;
    VxSdk::Utilities::StrCopySafe(filters[0].value, to_string(pageIndex).c_str());
    VxSdk::Utilities::StrCopySafe(filters[1].value, to_string(count).c_str());
    dataSources.filters = filters;

    VxSdk::VxResult::Value result = _system->GetDataSources(dataSources);
    if (result == VxSdk::VxResult::kInsufficientSize) {
        dataSources.collection = new VxSdk::IVxDataSource*[dataSources.collectionSize];
        totalItems = dataSources.collectionSize;
        result = _system->GetDataSources(dataSources);
        if (result == VxSdk::VxResult::kOK) {
            for (int i = 0; i < dataSources.collectionSize; i++)
                dataSourceList->push_back(new DataSource(dataSources.collection[i]));
        }
        delete[] dataSources.collection;
    }
    return dataSourceList;
}

CPPConsole::Export* CPPConsole::System::CreateExport(NewExport* newExport) {
    Export* exporter = nullptr;
    int size = newExport->GetClips()->size();
    VxSdk::VxNewExport vxExport;
    vxExport.clips = new VxSdk::VxNewExportClip[size];
    vxExport.clipSize = size;

    vxExport.format = VxSdk::VxExportFormat::Value(newExport->GetFormat());
    VxSdk::Utilities::StrCopySafe(vxExport.name, newExport->GetExportName());
    VxSdk::Utilities::StrCopySafe(vxExport.password, newExport->GetExportPassword());
    for (int i = 0; i < size; i++) {
        VxSdk::VxNewExportClip clip;
        VxSdk::Utilities::StrCopySafe(clip.dataSourceId, newExport->GetClips()->at(i)->GetDataSourceId());
        VxSdk::Utilities::StrCopySafe(clip.startTime, newExport->GetClips()->at(i)->GetStartTime());
        VxSdk::Utilities::StrCopySafe(clip.endTime, newExport->GetClips()->at(i)->GetEndTime());
        vxExport.clips[i] = clip;
    }

    VxSdk::IVxExport* exportItem = nullptr;
    VxSdk::VxResult::Value result = _system->CreateExport(vxExport, exportItem);

    if (result == VxSdk::VxResult::kOK)
        exporter = new Export(exportItem);

    delete[] vxExport.clips;

    return exporter;
}

list<CPPConsole::Export*>* CPPConsole::System::GetExports() {
    list<Export*>* exportsList = new list<Export*>();
    VxSdk::VxCollection<VxSdk::IVxExport**> exports;

    VxSdk::VxResult::Value result = _system->GetExports(exports);
    if (result == VxSdk::VxResult::kInsufficientSize) {
        exports.collection = new VxSdk::IVxExport*[exports.collectionSize];
        result = _system->GetExports(exports);
        if (result == VxSdk::VxResult::kOK) {
            for (int i = 0; i < exports.collectionSize; i++)
                exportsList->push_back(new Export(exports.collection[i]));
        }
        delete[] exports.collection;
    }
    return exportsList;
}

CPPConsole::Export* CPPConsole::System::GetExportDetails(const char* expName, const char* id) {
    VxSdk::VxCollection<VxSdk::IVxExport**> exports;
    Export* expToRet = nullptr;

    exports.filterSize = 1;
    VxSdk::VxCollectionFilter filters[1];
    filters->key = VxSdk::VxCollectionFilterItem::kName;
    VxSdk::Utilities::StrCopySafe(filters->value, expName);
    exports.filters = filters;

    VxSdk::VxResult::Value result = _system->GetExports(exports);
    if (result == VxSdk::VxResult::kInsufficientSize) {
        exports.collection = new VxSdk::IVxExport*[exports.collectionSize];
        result = _system->GetExports(exports);
        if (result == VxSdk::VxResult::kOK) {
            for (int i = 0; i < exports.collectionSize; i++) {
                VxSdk::IVxExport* exportObj = exports.collection[i];
                if (strcmp(exportObj->id, id) == 0)
                    expToRet = new Export(exports.collection[i]);
            }
        }
        delete[] exports.collection;
    }
    return expToRet;
}

bool CPPConsole::System::DeleteExport(Export* exportToDelete) {
    VxSdk::IVxExport* delExport = exportToDelete->GetExportPtr();
    VxSdk::VxResult::Value result = delExport->DeleteExport();
    if (result == VxSdk::VxResult::kOK)
        return true;

    return false;
}

bool CPPConsole::System::SubscribeSystemEvents(VxSdk::VxEventCallback callBack) {
    VxSdk::VxResult::Value result = _system->StartNotifications(callBack);
    return result == VxSdk::VxResult::kOK;
}

bool CPPConsole::System::UnSubscribeSystemEvents() {
    VxSdk::VxResult::Value result = _system->StopNotifications();
    return result == VxSdk::VxResult::kOK;
}

bool CPPConsole::System::InjectNewEvent(Situation* situation, string genDeviceId, string srcDeviceId, string eventTime) {
    VxSdk::VxNewEvent newEvent;
    VxSdk::Utilities::StrCopySafe(newEvent.situationType, situation->GetSituationType());
    VxSdk::Utilities::StrCopySafe(newEvent.generatorDeviceId, genDeviceId.c_str());
    VxSdk::Utilities::StrCopySafe(newEvent.sourceDeviceId, srcDeviceId.c_str());
    VxSdk::Utilities::StrCopySafe(newEvent.time, eventTime.c_str());

    VxSdk::VxResult::Value result = _system->InsertEvent(newEvent);
    return result == VxSdk::VxResult::kOK;
}

list<CPPConsole::Situation*>* CPPConsole::System::GetSituations() {
    list<Situation*>* situationsList = new list<Situation*>();
    VxSdk::VxCollection<VxSdk::IVxSituation**> situations;

    VxSdk::VxResult::Value result = _system->GetSituations(situations);
    if (result == VxSdk::VxResult::kInsufficientSize) {
        situations.collection = new VxSdk::IVxSituation*[situations.collectionSize];
        result = _system->GetSituations(situations);
        if (result == VxSdk::VxResult::kOK) {
            for (int i = 0; i < situations.collectionSize; i++)
                situationsList->push_back(new Situation(situations.collection[i]));
        }
        delete[] situations.collection;
    }
    return situationsList;
}

bool CPPConsole::System::AddNewSituation(const char* name, const char* type) {
    bool res = false;

    VxSdk::VxNewSituation situation;
    situation.Default();

    strncpy_s(situation.name, name, sizeof situation.name);
    strncpy_s(situation.type, type, sizeof situation.type);

    VxSdk::VxResult::Value result = _system->AddSituation(situation);
    if (result == VxSdk::VxResult::kOK)
        res = true;

    // Delete the memory allocated in the Default method
    delete[] situation.snoozeIntervals;

    return res;
}

bool CPPConsole::System::DeleteSituation(Situation* situationToDelete) {
    VxSdk::VxResult::Value result = situationToDelete->Self()->DeleteSituation();
    if (result == VxSdk::VxResult::kOK)
        return true;

    return false;
}

CPPConsole::System::~System() {
    if (_dataSourceList != nullptr) {
        for (list<DataSource*>::const_iterator iterator = _dataSourceList->begin(), end = _dataSourceList->end(); iterator != end; ++iterator) {
            delete *iterator;
        }
        delete _dataSourceList;
    }

    _system->Delete();
    _system = nullptr;
}
