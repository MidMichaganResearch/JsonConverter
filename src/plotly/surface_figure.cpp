#include "plotly/surface_figure.h"
#include "plotly/figure_parts.h"

namespace plotconv {

    namespace {
        // Sets the position of the play / stop buttons and the slider padding for surface plots.
        const ButtonsPlacement kButtonsPlacement{ 0.0, 35, "" };
        constexpr int kSliderTopPadding = 30;

        /// <summary>
        /// Writes the trace data for a surface plot to the JSON writer.
        /// </summary>
        /// <param name="w"> Instance of the JSON writer. </param>
        /// <param name="s"> Instance of the surface figure. </param>
        void writeSurfaceTrace(JsonWriter& w, const SurfaceFigure& s)
        {
            w.beginObject();

			// Sets the type of the trace to "surface".
            w.key("type");
            w.writeString("surface");

			// Sets the color scale for the surface plot to "Jet".
            w.key("colorscale");
            w.writeString("Jet");

			// Sets the minimum and maximum values for the color scale, which are used to map the data values to colors.
            w.key("cmin");
            w.writeNumberToken(s.colorMinToken);
            w.key("cmax");
            w.writeNumberToken(s.colorMaxToken);

			// Enables the color scale to be displayed on the right side of the plot.
            w.key("showscale");
            w.writeBool(true);

			// Sets the title and position of the color scale to be displayed on the right side of the plot, with a left edge at 102% of the plot width.
            w.key("colorbar");
            w.beginObject();
            w.key("x");
            w.writeNumber(1.02);
            writeTitle(w, s.zLabel);

			// Sets the number of ticks on the color scale to 10.
            w.key("nticks");
            w.writeInteger(10);
            w.endObject();

			// Sets the information displayed when hovering over a point on the surface.
            w.key("hovertemplate");
            w.writeString(
                s.xLabel + ": %{x}<br>" + s.yLabel + ": %{y}<br>" + s.zLabel + ": %{z}<extra></extra>");

			// Sets the z values shown when the plot opens: the whole surface when static, or the first frame when animated.
            w.key("z");
            w.writeRaw(s.firstFrameZ);

            w.endObject();
        }
    }

    /// <summary>
    /// Writes the complete surface plot file, including the trace data, layout information, and animation frames if applicable.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer. </param>
    /// <param name="surface"> Instance of the surface figure. </param>
    void writeSurfaceFigure(JsonWriter& w, const SurfaceFigure& surface)
    {
		// Determines whether the surface plot is animated based on the number of frames in the SurfaceFigure.
        const bool animated = surface.frameCount > 0;

        w.beginObject();

		// Sets the trace data for the surface plot, which includes the z-values, color scale, and hover information.
        w.key("data");
        w.beginArray();
        writeSurfaceTrace(w, surface);
        w.endArray();

		// Sets the layout information for the surface plot, which includes the title, axis labels, ranges, and aspect ratio.
        w.key("layout");
        w.beginObject();
        writeTitle(w, surface.title);
        writeScene(w, surface.xLabel, surface.xRange, surface.yLabel, surface.yRange, surface.zLabel, surface.zRange);
        
		// Verifies that if the surface plot is animated, it must add the controls for the animation, including the play/stop buttons and the slider for selecting frames.
        if (animated)
        {
            writePlayStopButtons(w, kButtonsPlacement);
            writeSlider(w, surface.frameCount, surface.sliderPrefix, kSliderTopPadding);
        }
        w.endObject();

		// Verifies that if the surface plot is animated, it must add the frames for the animation, which includes the z-values for each frame and the corresponding frame index.
        if (animated)
        {
            w.key("frames");
            w.writeRaw(surface.framesJson);
        }

        w.endObject();
    }
}
