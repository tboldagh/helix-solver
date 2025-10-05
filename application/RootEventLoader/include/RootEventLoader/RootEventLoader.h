#pragma once

#include "EventUsm/EventUsm.h"

#include <vector>
#include <string>
#include <tuple>
#include <memory>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>

class RootEventLoader
{
public:
    RootEventLoader() = default;
    ~RootEventLoader();

    bool setInputFile(const std::string& path);
    const std::vector<u_int32_t>& getEventIds();
    bool loadEvent(u_int32_t eventId, float* xs, float* ys, float* zs, u_int32_t* numPoints);
    std::tuple<bool, std::unique_ptr<EventUsm>> loadEvent(u_int32_t eventId);
    std::unique_ptr<std::map<u_int32_t, std::unique_ptr<EventUsm>>> loadAllEvents();

private:
    bool loadEventIds();

    std::string inputFilePath_;
    bool eventIdsLoaded_ = false;
    std::vector<u_int32_t> eventIds_;

    std::unique_ptr<TFile> file_;
    TTree* eventsTree_;
    TBranch* eventIdBranch_;
    TBranch* xBranch_;
    TBranch* yBranch_;
    TBranch* zBranch_;
};
