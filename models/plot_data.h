#pragma once

#include <string>
#include <vector>

enum class PlotType
{
    Scatter,
    Surface,
    Heatmap,
    Scatter3D
};

struct Series
{
    std::string name;

    std::vector<double> x;

    std::vector<double> y;

    std::vector<double> z;

    std::vector<std::vector<double>> matrix;
};

struct PlotData
{
    // default initialized to avoid C26495 uninitialized-member warning
    // (choose Scatter as a sensible default; adjust if needed)
    PlotType plotType = PlotType::Scatter;

    std::string title;    
    std::string subtitle;

    std::string xLabel;
    std::string yLabel;
    std::string zLabel;

    std::string units;

    // Scatter plots
    std::vector<Series> series;

    // Surface plots
    std::vector<std::vector<double>> matrix;

	// Animated plots (multiple frames)
    std::vector<std::vector<std::vector<double>>> frames;
};