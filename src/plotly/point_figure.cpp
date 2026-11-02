#include "plotly/point_figure.h"

#include "core/text_utils.h"
#include "plotly/figure_parts.h"

namespace plotconv {

    namespace {
		// Sets the position of the play / stop buttons and the slider padding for 3D point plots, below the plot.
        const ButtonsPlacement kButtonsPlacement{-0.05, 10, "left"};
        constexpr int kSliderTopPadding = 40;

        /// <summary>
        /// Writes the trace of the 3D point plot, showing the first frame.
        /// </summary>
        /// <param name="w"> Instance of the JSON writer. </param>
        /// <param name="f"> Instance of the point figure. </param>
        void writePointTrace(JsonWriter& w, const PointFigure& f)
        {
            w.beginObject();

			// Sets the trace as 3D points joined by a line.
            w.key("type");
            w.writeString("scatter3d");
            w.key("mode");
            w.writeString("lines+markers");
            w.key("name");
            w.writeString("Points");

			// Sets the coordinates of the first frame, and the color value of each point for the hover text.
            w.key("x");
            w.writeRaw(f.firstFrame.x);
            w.key("y");
            w.writeRaw(f.firstFrame.y);
            w.key("z");
            w.writeRaw(f.firstFrame.z);
            w.key("customdata");
            w.writeRaw(f.firstFrame.color);

			// Sets the information displayed when hovering over a point: its node number, coordinates and color value.
            w.key("hovertemplate");
            w.writeString("Node #: %{pointNumber}<br>" + f.xLabel + ": %{x}<br>" + f.yLabel + ": %{y}<br>" + f.zLabel
                + ": %{z}<br>" + f.colorLabel + ": %{customdata}" + f.colorUnits + "<extra></extra>");

			// Sets the line joining the points.
            w.key("line");
            w.beginObject();
            w.key("color");
            w.writeString("#378ADD");
            w.key("width");
            w.writeInteger(4);
            w.endObject();

			// Sets the markers, colored by their value with the "Jet" color scale between the extremes of every frame.
            w.key("marker");
            w.beginObject();
            w.key("size");
            w.writeInteger(7);
            w.key("color");
            w.writeRaw(f.firstFrame.color);
            w.key("colorscale");
            w.writeString("Jet");
            w.key("cmin");
            w.writeNumberToken(f.colorMinToken);
            w.key("cmax");
            w.writeNumberToken(f.colorMaxToken);
            w.key("showscale");
            w.writeBool(true);

			// Sets the color bar on the right side of the plot, with a left edge at 102% of the plot width and 10 ticks.
            w.key("colorbar");
            w.beginObject();
            w.key("x");
            w.writeNumber(1.02);
            writeTitle(w, labelWithUnits(f.colorLabel, f.colorUnits));
            w.key("nticks");
            w.writeInteger(10);
            w.endObject();

			// Sets a slightly transparent marker with a thin white border.
            w.key("opacity");
            w.writeNumber(0.95);
            w.key("line");
            w.beginObject();
            w.key("color");
            w.writeString("#ffffff");
            w.key("width");
            w.writeNumber(0.5);
            w.endObject();

            w.endObject();   // End configuration of the marker.

            w.endObject();
        }

        /// <summary>
        /// Writes the margins around the plot, leaving room under it for the buttons and the slider.
        /// </summary>
        void writeMargin(JsonWriter& w)
        {
            w.key("margin");
            w.beginObject();
            w.key("t");
            w.writeInteger(50);
            w.key("b");
            w.writeInteger(60);
            w.key("l");
            w.writeInteger(0);
            w.key("r");
            w.writeInteger(0);
            w.endObject();
        }
    }

    /// <summary>
    /// Writes the complete 3D point plot file, including the trace, the layout with its animation controls, and the frames.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer. </param>
    /// <param name="figure"> Instance of the point figure. </param>
    void writePointFigure(JsonWriter& w, const PointFigure& figure)
    {
        w.beginObject();

		// Sets the trace of the plot, showing the first frame.
        w.key("data");
        w.beginArray();
        writePointTrace(w, figure);
        w.endArray();

		// Sets the layout information, which includes the title, the 3D scene, the margins and the animation controls.
        w.key("layout");
        w.beginObject();
        writeTitle(w, figure.title);
        writeScene(w, figure.xLabel, figure.xRange, figure.yLabel, figure.yRange, figure.zLabel, figure.zRange);
        writeMargin(w);
        writePlayStopButtons(w, kButtonsPlacement);
        writeSlider(w, figure.frameCount, figure.sliderPrefix, kSliderTopPadding);
        w.endObject();

		// Sets the frames of the animation.
        w.key("frames");
        w.writeRaw(figure.framesJson);

        w.endObject();
    }

    /// <summary>
    /// Writes one frame of the animation: the coordinates of its points and their color values.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer, positioned inside the "frames" array. </param>
    /// <param name="index"> Index of the frame, used for its name </param>
    /// <param name="values"> The serialized values of the frame. </param>
    void writePointFrame(JsonWriter& w, size_t index, const PointValues& values)
    {
        w.beginObject();
        w.key("name");
        w.writeString("f" + std::to_string(index));
        w.key("traces");
        w.beginArray();
        w.writeInteger(0);
        w.endArray();

        w.key("data");
        w.beginArray();
        w.beginObject();
        w.key("x");
        w.writeRaw(values.x);
        w.key("y");
        w.writeRaw(values.y);
        w.key("z");
        w.writeRaw(values.z);
        w.key("customdata");
        w.writeRaw(values.color);
        w.key("marker");
        w.beginObject();
        w.key("color");
        w.writeRaw(values.color);
        w.endObject();
        w.endObject();
        w.endArray();

        w.endObject();
    }

}
