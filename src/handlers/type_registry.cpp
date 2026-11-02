#include "handlers/type_registry.h"

#include "handlers/matrix_frames_handler.h"
#include "handlers/single_matrix_handler.h"
#include "handlers/line_series_handler.h"
#include "handlers/point_frames_handler.h"

namespace plotconv {
    
    namespace {
        /// <summary>
        /// Creates the handlers for every supported raw file type.
        /// </summary>
        std::vector<std::unique_ptr<TypeHandler>> createHandlers()
        {
            std::vector<std::unique_ptr<TypeHandler>> handlers;

	        // Sets the handlers for the matrix frames in animated plots.
            for (const MatrixFramesType& type : matrixFramesTypes())
            {
                handlers.push_back(std::make_unique<MatrixFramesHandler>(type));
            }

	        // Sets the handlers for the single matrix types in static plots.
            for (const SingleMatrixType& type : singleMatrixTypes())
            {
                handlers.push_back(std::make_unique<SingleMatrixHandler>(type));
            }

            // Sets the handlers for the line series types in 2D line plots.
            for (const LineSeriesType& type : lineSeriesTypes())
            {
                handlers.push_back(std::make_unique<LineSeriesHandler>(type));
            }

            // Sets the handlers for the point frames types in animated 3D point plots.
            for (const PointFramesType& type : pointFramesTypes())
            {
                handlers.push_back(std::make_unique<PointFramesHandler>(type));
            }

            return handlers;
        }
    }

    /// <summary>
    /// Returns the handlers for every supported raw file type, created only once.
    /// </summary>
    const std::vector<std::unique_ptr<TypeHandler>>& registeredHandlers()
    {
        static const std::vector<std::unique_ptr<TypeHandler>> handlers = createHandlers();
        return handlers;
    }

}