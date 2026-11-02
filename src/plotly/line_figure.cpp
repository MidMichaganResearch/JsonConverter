#include "plotly/line_figure.h"

#include "plotly/figure_parts.h"

namespace plotconv {

    namespace {
        /// <summary>
        /// Writes one curve of the line plot.
        /// </summary>
        /// <param name="w"> Instance of the JSON writer. </param>
        /// <param name="figure"> Instance of the line figure. </param>
        /// <param name="series"> Index of the series to write. </param>
        void writeLineTrace(JsonWriter& w, const LineFigure& figure, size_t series)
        {
            const std::string& name = figure.seriesNames[series];

            w.beginObject();

			// Sets the trace as a scatter plot drawn with lines only.
            w.key("type");
            w.writeString("scatter");
            w.key("mode");
            w.writeString("lines");
            w.key("name");
            w.writeString(name);

			// Sets the values of the curve, the x values being shared by every curve of the plot.
            w.key("x");
            w.writeRaw(figure.xValues);
            w.key("y");
            w.writeRaw(figure.yValues[series]);

			// Sets the width of the line and the size of the markers.
            w.key("line");
            w.beginObject();
            w.key("width");
            w.writeInteger(2);
            w.endObject();
            w.key("marker");
            w.beginObject();
            w.key("size");
            w.writeInteger(2);
            w.endObject();

			// Sets the information displayed when hovering over a point of the curve.
            w.key("hovertemplate");
            w.writeString(figure.xLabel + ": %{x}<br>" + name + ": %{y}<extra></extra>");

            w.endObject();
        }

        /// <summary>
        /// Writes the title of a 2D axis to the JSON writer.
        /// </summary>
        void writeAxisTitle(JsonWriter& w, const char* name, const std::string& label)
        {
            w.key(name);
            w.beginObject();
            writeTitle(w, label);
            w.endObject();
        }
    }

    /// <summary>
    /// Writes the complete line plot file, including one trace per series and the layout with the title and axis labels.
    /// </summary>
    /// <param name="w"> Instance of the JSON writer. </param>
    /// <param name="figure"> Instance of the line figure. </param>
    void writeLineFigure(JsonWriter& w, const LineFigure& figure)
    {
        w.beginObject();

		// Sets one trace per series of the plot.
        w.key("data");
        w.beginArray();
        for (size_t series = 0; series < figure.yValues.size(); ++series)
        {
            writeLineTrace(w, figure, series);
        }
        w.endArray();

		// Sets the layout information for the line plot, which includes the title and axis labels.
        w.key("layout");
        w.beginObject();
        writeTitle(w, figure.title);
        writeAxisTitle(w, "xaxis", figure.xLabel);
        writeAxisTitle(w, "yaxis", figure.yLabel);
        w.endObject();

        w.endObject();
    }

}
