/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"
#include <QDebug>

#ifdef DEBUG_EXTRACTION
#   include "opentrack-compat/timer.hpp"
#endif

PointExtractor::PointExtractor(){
	//if (!AllocConsole()){}
	//else SetConsoleTitle("debug");
	//freopen("CON", "w", stdout);
	//freopen("CON", "w", stderr);
}
// ----------------------------------------------------------------------------
std::vector<cv::Vec2f> PointExtractor::extract_points(cv::Mat& frame)
{
    const int W = frame.cols;
    const int H = frame.rows;

    // convert to grayscale
    cv::Mat frame_gray;
    cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);

    int min_size = s.min_point_size;
    int max_size = s.max_point_size;
    
    unsigned int region_size_min = 3.14*min_size*min_size/4.0;
    unsigned int region_size_max = 3.14*max_size*max_size/4.0;
    
    // testing indicates threshold difference of 45 from lowest to highest
    // that's applicable to poor lighting conditions.
    
    static constexpr int diff = 20;
    static constexpr int steps = 5;
    static constexpr int successes = 5;
    
    int thres = s.threshold;
    
    struct blob {
        std::vector<cv::Vec2d> pos;
        std::vector<double> confids;
        std::vector<double> areas;
        
        cv::Vec2d effective_pos() const
        {
            double x = 0;
            double y = 0;
            double norm = 0;
            for (unsigned i = 0; i < pos.size(); i++)
            {
                const double w = confids[i] * areas[i];
                x += pos[i][0] * w;
                y += pos[i][1] * w;
                norm += w;
            }
            cv::Vec2d ret(x, y);
            ret *= 1./norm;
            return ret;
        }

        double effective_area() const
        {
            double norm = 0, ret = 0;
            for (unsigned i = 0; i < areas.size(); i++)
            {
                const double w = confids[i];
                norm += w;
                ret += w * areas[i];
            }
            return ret/norm;
        }
    };
    
    struct simple_blob
    {
        double radius_2;
        cv::Vec2d pos;
        double confid;
        bool taken;
        double area;
        simple_blob(double radius, const cv::Vec2d& pos, double confid, double area) : radius_2(radius*radius), pos(pos), confid(confid), taken(false), area(area)
        {
            //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1] << "confid" << confid;
        }
        bool inside(const simple_blob& other)
        {
            cv::Vec2d tmp = pos - other.pos;
            return tmp.dot(tmp) < radius_2;
        }
        static std::vector<blob> merge(cv::Mat& frame, std::vector<simple_blob>& blobs)
        {
#ifdef DEBUG_EXTRACTION
            static Timer t;
            bool debug = t.elapsed_ms() > 100;
            if (debug) t.start();
#endif
            
            std::vector<blob> ret;
            for (unsigned i = 0; i < blobs.size(); i++)
            {
                auto& b = blobs[i];
                if (b.taken)
                    continue;
                b.taken = true;
                blob b_;
                b_.pos.push_back(b.pos);
                b_.confids.push_back(b.confid);
                b_.areas.push_back(b.area);
                
                for (unsigned j = i+1; j < blobs.size(); j++)
                {
                    auto& b2 = blobs[j];
                    if (b2.taken)
                        continue;
                    if (b.inside(b2) || b2.inside(b))
                    {
                        b2.taken = true;
                        b_.pos.push_back(b2.pos);
                        b_.confids.push_back(b2.confid);
                        b_.areas.push_back(b2.area);
                    }
                }
                if (b_.pos.size() >= successes)
                    ret.push_back(b_);

                char buf[64];
                sprintf(buf, "%d%% %d px", (int)(b_.pos.size()*100/successes), (int)(2.*sqrt(b_.effective_area()) / sqrt(3.14)));
                const auto pos = b_.effective_pos();
                cv::putText(frame, buf, cv::Point(pos[0]+30, pos[1]+20), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 1);
            }
#ifdef DEBUG_EXTRACTION
            if (debug)
            {
                double diff = 0;
                for (unsigned j = 0; j < ret.size(); j++)
                {
                    auto& b = ret[j];
                    cv::Vec2d pos = b.effective_pos();
                    for (unsigned i = 0; i < b.pos.size(); i++)
                    {
                        auto tmp = pos - b.pos[i];
                        diff += std::abs(tmp.dot(tmp));
                    }
                }
                qDebug() << "diff" << diff;
            }
#endif
            return ret;
        }
    };
    
    // mask for everything that passes the threshold (or: the upper threshold of the hysteresis)
    cv::Mat frame_bin = cv::Mat::zeros(H, W, CV_8U);
    
    const int min = std::max(0, thres - diff/2);
    const int max = std::min(255, thres + diff/2);
    const int step = std::max(1, diff / steps);
    
    std::vector<simple_blob> blobs;
    
    // this code is based on OpenCV SimpleBlobDetector
    for (int i = min; i < max; i += step)
    {
        cv::Mat frame_bin_;
        cv::threshold(frame_gray, frame_bin_, i, 255, cv::THRESH_BINARY);
        frame_bin.setTo(170, frame_bin_);
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(frame_bin_, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        
        int cnt = 0;
        
        for (auto& c : contours)
        {
            if (cnt++ > 30)
                break;
            
            auto m = cv::moments(cv::Mat(c));
            const double area = m.m00;
            if (area == 0.)
                continue;
            cv::Vec2d pos(m.m10 / m.m00, m.m01 / m.m00);
            if (area < region_size_min || area > region_size_max)
                continue;
            
            double radius = 0;
            
            for (auto& k : c)
            {
                cv::Vec2d pos_(k.x, k.y);
                cv::Vec2d tmp = pos_ - pos;
                radius = std::max(radius, sqrt(1e-2 + tmp.dot(tmp)));
            }
            double confid = 1;
            {
                double denominator = std::sqrt(std::pow(2 * m.mu11, 2) + std::pow(m.mu20 - m.mu02, 2));
                const double eps = 1e-2;
                if (denominator > eps)
                {
                    double cosmin = (m.mu20 - m.mu02) / denominator;
                    double sinmin = 2 * m.mu11 / denominator;
                    double cosmax = -cosmin;
                    double sinmax = -sinmin;
    
                    double imin = 0.5 * (m.mu20 + m.mu02) - 0.5 * (m.mu20 - m.mu02) * cosmin - m.mu11 * sinmin;
                    double imax = 0.5 * (m.mu20 + m.mu02) - 0.5 * (m.mu20 - m.mu02) * cosmax - m.mu11 * sinmax;
                    confid = imin / imax;
                }
            }
            blobs.push_back(simple_blob(radius, pos, confid, area));
        }
    }
    
    // clear old points
	points.clear();
    
    for (auto& b : simple_blob::merge(frame, blobs))
    {
        auto pos = b.effective_pos();
        cv::Vec2f p((pos[0] - W/2)/W, -(pos[1] - H/2)/W);
        points.push_back(p);
    }
    
    std::vector<cv::Mat> channels_;
    cv::split(frame, channels_);
    // draw output image
    cv::Mat frame_bin_ = frame_bin * .5;
    std::vector<cv::Mat> channels;
    channels.push_back(channels_[0] + frame_bin_);
    channels.push_back(channels_[1] - frame_bin_);
    channels.push_back(channels_[2] - frame_bin_);
    cv::merge(channels, frame);

    return points;
}
