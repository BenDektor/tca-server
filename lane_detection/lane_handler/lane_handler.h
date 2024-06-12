#ifndef LANE_HANDLER_H
#define LANE_HANDLER_H

#include "../lane_finder/Strasenfinder2.h" // Assuming the header file where StrasenFinder2 class is defined

enum CarPosition {
    NO_STREET = 0,
    ON_STREET = 1,
    OFF_STREET = 2
};


class LaneHandler {
public:
    CarPosition getCarPosition(cv::Mat image);

private:
    StrasenFinder2 strassenFinder;

    CarPosition handleOneLane(const std::vector<LineProperties>& lineGroup);
    CarPosition handleTwoLanes(const std::vector<LineProperties>& lineGroup1, const std::vector<LineProperties>& lineGroup2);
    

    LineProperties calculateAverageLineProperties(const std::vector<LineProperties>& lines);

};



#endif // LANE_HANDLER_H
