#include "RootEventLoader/RootEventLoader.h"
#include "Logger/Logger.h"
#include "EventUsm/EventUsm.h"

#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TList.h>
#include <algorithm>
#include <set>


RootEventLoader::~RootEventLoader()
{
    if (!file_)
    {
        LOG_WARNING("File not opened");
        return;
    }

    file_->Close();
}

bool RootEventLoader::setInputFile(const std::string& path)
{
    inputFilePath_ = path;

    file_ = std::unique_ptr<TFile>(TFile::Open(inputFilePath_.c_str()));
    if (!file_)
    {
        LOG_WARNING("Failed to open file " + inputFilePath_);
        return false;
    }

    return loadEventIds();
}

bool RootEventLoader::loadEvent(u_int32_t eventId, float* xs, float* ys, float* zs, u_int32_t* numPoints)
{
    if (!eventIdsLoaded_ && !loadEventIds())
    {
        return false;
    }

    if (std::find(eventIds_.begin(), eventIds_.end(), eventId) == eventIds_.end())
    {
        LOG_WARNING("Event id " + std::to_string(eventId) + " not found in " + inputFilePath_);
        return false;
    }

    u_int32_t dataEventId;
    float x;
    float y;
    float z;
    eventIdBranch_->SetAddress(&dataEventId);
    xBranch_->SetAddress(&x);
    yBranch_->SetAddress(&y);
    zBranch_->SetAddress(&z);

    u_int32_t numEntries = eventsTree_->GetEntries();
    for (u_int32_t i = 0; i < numEntries; i++)
    {
        eventsTree_->GetEntry(i);
        if (dataEventId != eventId)
        {
            continue;
        }

        xs[*numPoints] = x;
        ys[*numPoints] = y;
        zs[*numPoints] = z;
        (*numPoints)++;
    }
    
    return true;
}

std::tuple<bool, std::unique_ptr<EventUsm>> RootEventLoader::loadEvent(u_int32_t eventId)
{
    std::unique_ptr<EventUsm> event = std::make_unique<EventUsm>(eventId);
    if (!loadEvent(eventId, event->hostXs_, event->hostYs_, event->hostZs_, &event->hostNumPoints_))
    {
        return std::make_tuple(false, nullptr);
    }
    
    return std::make_tuple(true, std::move(event));
}

std::unique_ptr<std::map<u_int32_t, std::unique_ptr<EventUsm>>> RootEventLoader::loadAllEvents()
{
    if (!file_)
    {
        LOG_WARNING("File not opened");
        return nullptr;
    }

    if (!eventIdsLoaded_ && !loadEventIds())
    {
        LOG_WARNING("Failed to load event ids");
        return nullptr;
    }

    std::unique_ptr<std::map<u_int32_t, std::unique_ptr<EventUsm>>> events = std::make_unique<std::map<u_int32_t, std::unique_ptr<EventUsm>>>();
    for (u_int32_t eventId : eventIds_)
    {
        events->emplace(eventId, std::make_unique<EventUsm>(eventId));
    }

    u_int32_t eventId;
    float x;
    float y;
    float z;
    eventIdBranch_->SetAddress(&eventId);
    xBranch_->SetAddress(&x);
    yBranch_->SetAddress(&y);
    zBranch_->SetAddress(&z);

    u_int64_t numEntries = eventsTree_->GetEntries();
    for (u_int64_t i = 0; i < numEntries; i++)
    {
        eventsTree_->GetEntry(i);
        auto& event = *events->at(eventId);
        event.hostXs_[event.hostNumPoints_] = x;
        event.hostYs_[event.hostNumPoints_] = y;
        event.hostZs_[event.hostNumPoints_] = z;
        event.hostLayers_[event.hostNumPoints_] = 0;
        event.hostNumPoints_++;
    }

    return events;
}

const std::vector<u_int32_t>& RootEventLoader::getEventIds()
{
    if (!eventIdsLoaded_)
    {
        loadEventIds();
    }

    return eventIds_;
}

bool RootEventLoader::loadEventIds()
{
    LOG_INFO("Loading event ids from " + inputFilePath_);

    if (!file_)
    {
        LOG_WARNING("File not opened");
        return false;
    }

    eventsTree_ = file_->Get<TTree>("spacepoints");
    if (!eventsTree_)
    {
        LOG_WARNING("Failed to get events tree from " + inputFilePath_);
        return false;
    }
    
    eventIdBranch_ = eventsTree_->GetBranch("event_id");
    if (!eventIdBranch_)
    {
        LOG_WARNING("Failed to get event_id branch from " + inputFilePath_);
        return false;
    }

    xBranch_ = eventsTree_->GetBranch("x");
    if (!xBranch_)
    {
        LOG_WARNING("Failed to get x branch from " + inputFilePath_);
        return false;
    }

    yBranch_ = eventsTree_->GetBranch("y");
    if (!yBranch_)
    {
        LOG_WARNING("Failed to get y branch from " + inputFilePath_);
        return false;
    }   

    zBranch_ = eventsTree_->GetBranch("z");
    if (!zBranch_)
    {
        LOG_WARNING("Failed to get z branch from " + inputFilePath_);
        return false;
    }

    u_int32_t eventId;
    eventIdBranch_->SetAddress(&eventId);
    std::set<u_int32_t> uniqueEventIds;
    u_int64_t numEntries = eventsTree_->GetEntries();
    for (u_int64_t i = 0; i < numEntries; i++)
    {
        eventsTree_->GetEntry(i);
        uniqueEventIds.insert(eventId);
    }

    eventIds_ = std::vector<u_int32_t>(uniqueEventIds.begin(), uniqueEventIds.end());
    std::sort(eventIds_.begin(), eventIds_.end());
    eventIdsLoaded_ = true;
    return true;
}
