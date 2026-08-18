#include "pressure_conform_parser.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>


PressureConformParser::PressureConformParser(
    const std::string& path)
    :
    filepath(path)
{
}

PlotData PressureConformParser::parse()
{
    PlotData data;
    std::ifstream file(filepath);
    std::string line;
    constexpr int HEADER_LINES = 5;
    int COLUMNS = 0;
    int ROWS_PER_FRAME=101;             //read the nodes from the header


   
    if (!file.is_open())  return data;        //Not open, so return
    data.plotType = PlotType::Surface;

    data.xLabel =        "Circumferential Node";
    data.yLabel =        "Radial Side Node";
    data.zLabel =        "Pressure";
	data.units = "PSI";

     for (int i = 0; i < HEADER_LINES; i++)
    {
        std::getline(file, line);
        if (i == 0) {
            std::string old_word = "THIS FILE CONTAINS THE";
            std::string new_word = "";

            // Find the starting position of the target word
            size_t pos = line.find(old_word);

            // Check if the word was actually found
            if (pos != std::string::npos) {
                // Parameters: (starting position, character count to erase, replacement string)
                line.replace(pos, old_word.length(), new_word);
            }

            //Set title
            data.title = line;
            data.subtitle = "";

        }
        if (i == 2) {
            //Probably better to read the data for the axis labels
        }
        if (i == 4) {
            //get the number of nodes off the data line
            try {
                // Parse the integer from the beginning of the string
                int number = std::stoi(line);
 //               std::cout << "Successfully parsed: " << number << std::endl;
                COLUMNS = number; //
            }
            catch (const std::invalid_argument& e) {
                std::cout << "No valid conversion could be performed." << std::endl;
                return data;
            }
            catch (const std::out_of_range& e) {
                std::cout << "The converted value falls out of the int range." << std::endl;
                return data;
            }
        }
    }

	 while (true)
	 {
		 std::vector<std::vector<double>> frame;
		 bool frameRead = false;
		 for (int row = 0;
			 row < ROWS_PER_FRAME;
			 row++)
		 {
			 if (!std::getline(file, line)) break;
			 if (line.empty())
			 {
				 std::cout
					 << "WARNING: Empty line found in frame "
					 << data.frames.size()
					 << " row "
					 << row
					 << std::endl;
				 continue;
			 }
			 frameRead = true;
			 for (char& c : line)    //strip out comma's in input, and change floading point to use the "E" notation
			 {
				 if (c == ',') c = ' ';
				 if (c == 'D') c = 'E';
			 }
			 std::stringstream ss(line);
			 std::vector<double> values;
			 double value;
			 while (ss >> value)
			 {
				 values.push_back(value);
			 }
			 if (values.size() != COLUMNS)
			 {
				 std::cout
					 << "BAD ROW in frame "
					 << data.frames.size()
					 << " row "
					 << row
					 << " values="
					 << values.size()
					 << std::endl;
				 std::cout
					 << "LINE: "
					 << line
					 << std::endl;
				 continue;
			 }
			 frame.push_back(values);
		 }
		 if (!frameRead)
		 {
			 break;
		 }
		 /*
		 ============================================================
		 FRAME VALIDATION
		 ============================================================
		 */
		 if (frame.size() != ROWS_PER_FRAME)
		 {
			 std::cout
				 << "BAD FRAME "
				 << data.frames.size()
				 << " size="
				 << frame.size()
				 << std::endl;
			 break;
		 }
		 double minX = frame[0][0];
		 double maxX = frame[0][0];
		 double minY = frame[0][1];
		 double maxY = frame[0][1];
		 double minZ = frame[0][2];
		 double maxZ = frame[0][2];
		 for (const auto& node : frame)
		 {
			 minX = std::min(minX, node[0]);
			 maxX = std::max(maxX, node[0]);
			 minY = std::min(minY, node[1]);
			 maxY = std::max(maxY, node[1]);

			 minZ = std::min(minZ, node[2]);
			 maxZ = std::max(maxZ, node[2]);
		 }
		 /*
		 //Trap suspicious node values???Maybe not necessary???DSB
		 if (std::abs(maxX) > 100.0 ||
			 std::abs(minX) > 100.0 ||
			 std::abs(maxY) > 100.0 ||
			 std::abs(minY) > 100.0 ||
			 std::abs(maxZ) > 100.0 ||
			 std::abs(minZ) > 100.0)
		 {
			 std::cout
				 << "SUSPICIOUS FRAME "
				 << data.frames.size()
				 << std::endl;
		 }
		 for (size_t node = 0; node < frame.size(); node++)
		 {
			 if (std::abs(frame[node][0]) > 10.0 ||
				 std::abs(frame[node][1]) > 10.0 ||
				 std::abs(frame[node][2]) > 10.0)
			 {
				 std::cout
					 << "BAD FRAME "
					 << data.frames.size()
					 << " NODE "
					 << node
					 << std::endl;
			 }
		 }
		 */
		 data.frames.push_back(frame);
		 /*
		 ===========================================================
		 SAMPLE OUTPUt
		 ===========================================================
		 if (data.frames.size() <= 5)
		 {
			 std::cout
				 << "\nFRAME "
				 << data.frames.size() - 1
				 << std::endl;
			 for (size_t i = 0; i < frame.size(); i++)
			 {
				 std::cout
					 << "Node "
					 << i
					 << " : "
					 << frame[i][0]
					 << ", "
					 << frame[i][1]
					 << ", "
					 << frame[i][2]
					 << std::endl;
			 }

			 std::cout
				 << "X Range: "
				 << minX
				 << " -> "
				 << maxX
				 << std::endl;

			 std::cout
				 << "Y Range: "
				 << minY
				 << " -> "
				 << maxY
				 << std::endl;

			 std::cout
				 << "Z Range: "
				 << minZ
				 << " -> "
				 << maxZ
				 << std::endl;
		 }
		 */
	 }
	 /*
	 std::cout
		 << "\nTOTAL FRAMES = "
		 << data.frames.size()
		 << std::endl;
	 if (!data.frames.empty())
	 {
		 std::cout
			 << "NODES PER FRAME = "
			 << data.frames[0].size()
			 << std::endl;
	 }
	  */

	 return data;
}