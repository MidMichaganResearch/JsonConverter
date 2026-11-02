#include "plotly/figure_parts.h"

namespace plotconv {

    namespace {
		// Sets the play and stop button labels using Unicode characters for the play (▶) and stop (■) symbols.
        const char* const kPlayLabel = "\xE2\x96\xB6";
        const char* const kStopLabel = "\xE2\x96\xA0";

		// Sets the duration of each frame in milliseconds when the animation plays.
        constexpr int kPlayFrameDurationMs = 50;

        /// <summary>
        /// Writes the label, range, and autorange setting of the given axis to the JSON writer.
        /// </summary>
        /// <param name="w"> Instance of the JSON writer. </param>
        /// <param name="name"> Name of the axis. </param>
        /// <param name="label"> Label for the axis. </param>
        /// <param name="range"> Range for the axis. </param>
        void writeAxis(JsonWriter& w, const char* name, const std::string& label, const AxisRange& range)
        {
            w.key(name);
            w.beginObject();

			// Sets the title of the axis using the provided label.
            writeTitle(w, label);

			// Sets the range of the axis using an array with the minimum and maximum values.
            w.key("range");
            w.beginArray();
            w.writeNumber(range.min);
            w.writeNumber(range.max);
            w.endArray();

			// Disables autorange for the axis to ensure that the specified range is used.
            w.key("autorange");
            w.writeBool(false);

            w.endObject();
        }

        /// <summary>
        /// Writes the transition and frame settings for the animation to the JSON writer.
        /// </summary>
        /// <param name="w"> Instance of the JSON writer. </param>
        /// <param name="frameDurationMs"> Duration of each frame in milliseconds. </param>
        /// <param name="redraw"> Boolean indicating whether to redraw or not the entire plot after each frame. </param>
        void writeTransitionAndFrame(JsonWriter& w, int frameDurationMs, bool redraw)
        {
			// Sets the animation mode to "immediate", so the new animation interrupts any running one and clears queued frames.
            w.key("mode");
            w.writeString("immediate");

			// Sets the transition duration to 0 ms, so frames switch instantly with no interpolation between them.
            w.key("transition");
            w.beginObject();
            w.key("duration");
            w.writeInteger(0);
            w.endObject();

            w.key("frame");
            w.beginObject();

			// Sets the duration of each frame in milliseconds for the animation.
            w.key("duration");
            w.writeInteger(frameDurationMs);

			// Sets whether to redraw the entire plot after each frame.
            w.key("redraw");
            w.writeBool(redraw);

            w.endObject();
        }
    }

    /// <summary>
    /// Writes the title of the plot to the JSON writer.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer. </param>
    /// <param name="text"> Text for the title of the plot. </param>
    void writeTitle(JsonWriter& w, const std::string& text)
    {
        w.key("title");
        w.beginObject();
        w.key("text");
        w.writeString(text);
        w.endObject();
    }

    /// <summary>
    /// Writes the scene information including camera position, axis labels, ranges, and aspect ratio, to the JSON writer.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer. </param>
    void writeScene(JsonWriter& w,
                    const std::string& xLabel, const AxisRange& xRange,
                    const std::string& yLabel, const AxisRange& yRange,
                    const std::string& zLabel, const AxisRange& zRange)
    {
        w.key("scene");
        w.beginObject();

		// Sets the camera position for the 3D plot, with the eye positioned at (1.0, 1.0, 1.0).
        w.key("camera");
        w.beginObject();
        w.key("eye");
        w.beginObject();
        w.key("x");
        w.writeNumber(1.0);
        w.key("y");
        w.writeNumber(1.0);
        w.key("z");
        w.writeNumber(1.0);
        w.endObject();
        w.endObject();

		// Sets the labels and ranges for the x, y, and z axes.
        writeAxis(w, "xaxis", xLabel, xRange);
        writeAxis(w, "yaxis", yLabel, yRange);
        writeAxis(w, "zaxis", zLabel, zRange);

		// Sets the aspect mode to manual.
        w.key("aspectmode");
        w.writeString("manual");

		// Sets the aspect ratio for the x, y, and z axes to 1:1:1.
        w.key("aspectratio");
        w.beginObject();
        w.key("x");
        w.writeInteger(1);
        w.key("y");
        w.writeInteger(1);
        w.key("z");
        w.writeInteger(1);
        w.endObject();

        w.endObject();
    }

    /// <summary>
    /// Writes the play and stop buttons for the animation to the JSON writer.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer. </param>
    /// <param name="placement"> Position and padding of the buttons under the plot. </param>
    void writePlayStopButtons(JsonWriter& w, const ButtonsPlacement& placement)
    {
        w.key("updatemenus");
        w.beginArray();
        w.beginObject();

		// Sets the buttons for the animation controls to be displayed horizontally, at the left edge of the plot.
        w.key("type");
        w.writeString("buttons");
        w.key("direction");
        w.writeString("left");
        w.key("showactive");
        w.writeBool(false);
        w.key("x");
        w.writeNumber(0.0);
        w.key("y");
        w.writeNumber(placement.y);

		// Sets the horizontal anchor of the buttons, only when the plot type asks for one.
        if (!placement.xAnchor.empty())
        {
            w.key("xanchor");
            w.writeString(placement.xAnchor);
        }

        w.key("yanchor");
        w.writeString("top");

		// Sets the padding for the buttons, with the plot type's top padding and a right padding of 10 pixels.
        w.key("pad");
        w.beginObject();
        w.key("t");
        w.writeInteger(placement.topPadding);
        w.key("r");
        w.writeInteger(10);
        w.endObject();

		// Specific settings for the Play / Stop buttons.
        w.key("buttons");
        w.beginArray();

		// Sets the label, method, and arguments for the Play button.
        w.beginObject();
        w.key("label");
        w.writeString(kPlayLabel);
        w.key("method");
        w.writeString("animate");
        w.key("args");
        w.beginArray();
        w.writeNull();
        w.beginObject();
        w.key("fromcurrent");
        w.writeBool(true);
        writeTransitionAndFrame(w, kPlayFrameDurationMs, true);
        w.endObject();
        w.endArray();
        w.endObject();

		// Sets the label, method, and arguments for the Stop button.
        w.beginObject();
        w.key("label");
        w.writeString(kStopLabel);
        w.key("method");
        w.writeString("animate");
        w.key("args");
        w.beginArray();
        w.beginArray();
        w.writeNull();
        w.endArray();
        w.beginObject();
        writeTransitionAndFrame(w, 0, false);
        w.endObject();
        w.endArray();
        w.endObject();

		w.endArray();   // End configuration of the buttons.
        w.endObject();
		w.endArray();   // End configuration of the animation controls.
    }

    /// <summary>
    /// Writes the slider control for the animation to the JSON writer.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer. </param>
    /// <param name="frameCount"> Number of frames in the animation. </param>
    /// <param name="prefix"> Text shown before the current step, e.g. "Crank Angle: ". </param>
    /// <param name="topPadding"> Pixels above the slider. </param>
    void writeSlider(JsonWriter& w, size_t frameCount, const std::string& prefix, int topPadding)
    {
        w.key("sliders");
        w.beginArray();
        w.beginObject();

		// Sets the slider to start at the first step (frame 0).
        w.key("active");
        w.writeInteger(0);

		// Sets the padding for the slider, with the plot type's top padding and a bottom padding of 10 pixels.
        w.key("pad");
        w.beginObject();
        w.key("t");
        w.writeInteger(topPadding);
        w.key("b");
        w.writeInteger(10);
        w.endObject();

		// Sets the position and length of the slider, with the left edge at 5% of the plot width and a length of 90% of the plot width.
        w.key("x");
        w.writeNumber(0.05);
        w.key("len");
        w.writeNumber(0.9);

		// Sets the current value of the slider, including the prefix text and formatting of it.
        w.key("currentvalue");
        w.beginObject();
        w.key("prefix");
        w.writeString(prefix);
        w.key("visible");
        w.writeBool(true);
        w.key("xanchor");
        w.writeString("center");
        w.key("font");
        w.beginObject();
        w.key("size");
        w.writeInteger(12);
        w.endObject();
        w.endObject();

		// Settings for the steps of the slider.
        w.key("steps");
        w.beginArray();

		// Iterates through each frame in the animation.
        for (size_t i = 0; i < frameCount; ++i)
        {
            const std::string index = std::to_string(i);

			// Sets the label, method, and arguments for the step of the slider.
            w.beginObject();
            w.key("label");
            w.writeString(index);
            w.key("method");
            w.writeString("animate");
            w.key("args");
            w.beginArray();

			// Sets the frame to be displayed when the slider is moved to this step, using the frame index as the label.
            w.beginArray();
            w.writeString("f" + index);
            w.endArray();

			// Sets the transition and frame settings for the animation when the slider is moved to this step.
            w.beginObject();
            writeTransitionAndFrame(w, 0, true);
            w.endObject();

            w.endArray();
            w.endObject();
        }
        w.endArray();   // End configuration of steps.
        w.endObject();
        w.endArray();   // End configuration of sliders.
    }

}
