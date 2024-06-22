#ifndef LANE_HANDLER_H
#define LANE_HANDLER_H

#include "../lane_finder/Strasenfinder2.h" // Assuming the header file where StrasenFinder2 class is defined

enum CarPosition
{
    NO_STREET = 0,
    ON_STREET = 1,
    OFF_STREET_TO_LEFT = 2,
    OFF_STREET_TO_RIGHT = 3,
    UNKNOWN = 4,
};

class LaneHandler
{
public:
    CarPosition getCarPosition(cv::Mat image);

    int get_steering_dir();
    int get_distance_to_street();
    int get_angle_to_street();
    cv::Mat getModifiedImage();

private:
    StrasenFinder2 strassenFinder;

    CarPosition handleOneLane(const std::vector<LineProperties> &lineGroup);
    CarPosition handleTwoLanes(const std::vector<LineProperties> &lineGroup1, const std::vector<LineProperties> &lineGroup2);

    LineProperties calculateAverageLineProperties(const std::vector<LineProperties> &lines);
    LineProperties calculateAverageLinePropertiesOfAverages(const std::vector<LineProperties> &lines);

    bool checkCarOnStreet(const LineProperties &line1, const LineProperties &line2);
    bool checkCarOnStreet(const LineProperties& line);
    
    int calculateSteeringDir(const LineProperties &line);
    std::pair<int, float> calculateDistanceAndAngleToStreet(const LineProperties &line);

    void drawLine(const LineProperties &line, const std::string &color, int strength = 2);
    void drawOffsetLines(int xValue);
    void drawVerticalHelperLine();
    void drawCarPositionText(CarPosition position); 
    cv::Mat drawing_image;

    int steering_dir;
    int distance_to_street;
    float angle_to_street;

    CarPosition temp_carPosition = CarPosition::UNKNOWN;
};

#endif // LANE_HANDLER_H
