#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <iostream>

void displaySelectInputType();
void displaySelectVideoSource();
void displayInsertCustomVideoSource();
void displaySelectChessboardDimensions();
void displayChessboardDimensions();
void displayInsertChessboardHeight();
void displayInsertChessboardWidth();
void displayLowFrameCountWarning();
void displayLowFrameCountError();
void displaySelectCapturingMode();

bool waiting_input = true;
uint8_t input_type;
std::string media_path;
std::ostringstream captured_frames_msg;

// Defining the dimensions of CHESSBOARD
int CHESSBOARD[2] = {8, 11};

bool manual_capture;

int main(int argc, char *argv[])
{
  // Get input source from user.
  cv::VideoCapture cap;
  waiting_input = true;
  char c;
  do
  {
    displaySelectInputType();
    std::cin >> c;
    switch (c)
    {
    case 49:
      // std::cout << "\nCamera stream selected." << std::endl;
      input_type = 1;
      break;
    case 50:
      // std::cout << "\nVideo file selected." << std::endl;
      input_type = 2;
      break;
    case 51:
      // std::cout << "\nPictures selected." << std::endl;
      input_type = 3;
      break;
    case 113:
      return -1;
      break;
    default:
      // std::cout << "\nInvalid option.\nTry again..." << std::endl;
      waiting_input = false;
      break;
    }
  } while (!waiting_input);

  // Get matrix dimensions from user.
  waiting_input = true;
  //  char c;
  do
  {
    displaySelectChessboardDimensions();
    std::cin >> c;
    switch (c)
    {
    case 49:
      std::cout << "Default size selected." << std::endl;
      waiting_input = false;
      break;
    case 50:
      do
      {
        displayInsertChessboardHeight();
        std::cin >> CHESSBOARD[0];
        if (CHESSBOARD[0] > 2)
        {
          waiting_input = false;
        }
        else
        {
          std::cout << "Incorrect value.\nTry again..." << std::endl;
        }
      } while (waiting_input);
      do
      {
        displayInsertChessboardWidth();
        std::cin >> CHESSBOARD[1];
        if (CHESSBOARD[1] > 2)
        {
          waiting_input = false;
        }
        else
        {
          std::cout << "Incorrect value.\nTry again..." << std::endl;
        }
      } while (waiting_input);
      break;
    default:
      std::cout << "\nInvalid option.\nTry again..." << std::endl;
      break;
    }
  } while (waiting_input);

  // Check if capture is done manually or automatically
  // Open choosen video input
  if (input_type == 1 || input_type == 2)
  {
    waiting_input = true;
    do
    {
      displaySelectCapturingMode();
      std::cin >> c;
      switch (c)
      {
      case 49:
        std::cout << "Manual capturing mode selected." << std::endl;
        manual_capture = true;
        waiting_input = false;
        break;
      case 50:
        std::cout << "Automatic capturing mode selected." << std::endl;
        manual_capture = false;
        waiting_input = false;
        break;
      default:
        std::cout << "\nInvalid option.\nTry again..." << std::endl;
        break;
      }
    } while (waiting_input);
  }

  // Open choosen video input
  waiting_input = true;
  switch (input_type)
  {
  case 1:
    std::cout << "Opening video stream." << std::endl;
    cap.open(0);
    if (!cap.isOpened())
    {
      std::cout << "Error opening video stream." << std::endl;
      return -1;
    }
    break;
  case 2:
    do
    {
      displaySelectVideoSource();
      // char c;
      std::cin >> c;
      switch (c)
      {
      case 49:
        std::cout << "Default location selected." << std::endl;
        media_path = "./videos/calib.mp4";
        waiting_input = false;
        break;
      case 50:
        displayInsertCustomVideoSource();
        std::cin >> media_path;
        waiting_input = false;
        break;
      default:
        std::cout << "\nInvalid option.\nTry again..." << std::endl;
        break;
      }
    } while (waiting_input);

    std::cout << "Opening video file." << std::endl;
    cap.open(media_path);
    // Check if camera opened successfully
    if (!cap.isOpened())
    {
      std::cout << "Error opening video file." << std::endl;
      return -1;
    }
    break;
  case 3:

    break;
  default:

    break;
  }

  // Matrices to save frames from video and processed frames
  cv::Mat frame, gray;
  // Creating vector to store vectors of 3D points for each CHESSBOARD image
  std::vector<std::vector<cv::Point3f>> objpoints;
  // Creating vector to store vectors of 2D points for each CHESSBOARD image
  std::vector<std::vector<cv::Point2f>> imgpoints;
  // Defining the world coordinates for 3D points
  std::vector<cv::Point3f> objp;
  // Initialize chessboard corners
  for (int i{0}; i < CHESSBOARD[1] - 1; i++)
  {
    for (int j{0}; j < CHESSBOARD[0] - 1; j++)
      objp.push_back(cv::Point3f(j, i, 0));
  }
  // Vector to store the pixel coordinates of detected checker board corners
  std::vector<cv::Point2f> corner_pts;
  // Set true if chessboard is detected.
  bool success;
  // Counts the number of captured calibration frames
  int calib_frames_count = 0;
  captured_frames_msg.str("Captured frames: 0");

  uint8_t font_color[3] = {0, 0, 255};
  uint8_t font_scale = 1;

  int delay = 2 * (CLOCKS_PER_SEC * 1.54);
  std::clock_t now = std::clock();

  while (1)
  {
    // Capture frame-by-frame
    cap >> frame;

    // If the frame is empty, break immediately
    if (frame.empty())
      break;

    // Display the resulting frame
    cv::imshow("Input", frame);

    // Press ESC on keyboard to exit,
    c = (char)cv::waitKey(17);

    if (c == 27)
    {
      if (calib_frames_count < 10)
      {
        do
        {
          displayLowFrameCountError();
          std::cin >> c;
          switch (c)
          {
          case 49:
            std::cout << "Good decision." << std::endl;
            waiting_input = false;
            break;
          case 50:
            std::cout << "Exiting calibration." << std::endl;
            waiting_input = false;
            goto quit;
            break;
          default:
            std::cout << "\nInvalid option.\n"
                      << "Try again..." << std::endl;
            break;
          }
        } while (waiting_input);
      }
      else if (calib_frames_count < 20)
      {
        do
        {
          displayLowFrameCountWarning();
          std::cin >> c;
          switch (c)
          {
          case 49:
            std::cout << "Good decision." << std::endl;
            waiting_input = false;
            break;
          case 50:
            std::cout << "Proceed the calibration with few samples." << std::endl;
            waiting_input = false;
            goto force_capture_end;
            break;
          default:
            std::cout << "\nInvalid option.\n"
                      << "Try again..." << std::endl;
            break;
          }
        } while (waiting_input);
      }
      else
      {
      force_capture_end:
        break;
      }
    }

    // Convert image to grayscale
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    // Finding checker board corners
    // If desired number of corners are found in the image then success = true
    success = cv::findChessboardCorners(gray, cv::Size(CHESSBOARD[0] - 1, CHESSBOARD[1] - 1), corner_pts, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

    /*
     * If desired number of corner are detected,
     * we refine the pixel coordinates and display
     * them on the images of checker board
     */
    if (success)
    {
      cv::TermCriteria criteria(cv::TermCriteria::MAX_ITER | cv::TermCriteria::EPS, 30, 0.001);

      // refining pixel coordinates for given 2d points.
      cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

      // Displaying the detected corner points on the checker board
      cv::drawChessboardCorners(frame, cv::Size(CHESSBOARD[0] - 1, CHESSBOARD[1] - 1), corner_pts, success);

      if (manual_capture)
      {
        if (c == 99)
        {
        capture_points:
          objpoints.push_back(objp);
          imgpoints.push_back(corner_pts);
          calib_frames_count++;
          std::cout << "Calibration data captured." << std::endl;

          captured_frames_msg.str("");
          captured_frames_msg << "Captured frames: " << calib_frames_count;

          if (calib_frames_count < 10)
          {
            font_color[0] = 0;
            font_color[1] = 0;
            font_color[2] = 255;
          }
          else if (calib_frames_count < 15)
          {
            font_color[0] = 0;
            font_color[1] = 125;
            font_color[2] = 255;
          }
          else if (calib_frames_count < 20)
          {
            font_color[0] = 0;
            font_color[1] = 255;
            font_color[2] = 255;
          }
          else if (calib_frames_count < 30)
          {
            font_color[0] = 0;
            font_color[1] = 255;
            font_color[2] = 165;
          }
          else
          {
            font_color[0] = 0;
            font_color[1] = 255;
            font_color[2] = 0;
          }
        }
      }
      else
      {
        if (std::clock() - now >= delay)
        {
          now = std::clock();
          goto capture_points;
        }
      }
    }

    std::string cap_frame_msg = captured_frames_msg.str();
    cv::putText(frame, cap_frame_msg, cv::Point(25, 25), cv::FONT_HERSHEY_COMPLEX_SMALL, font_scale, cv::Scalar(font_color[0], font_color[1], font_color[2]), 1.8, false);

    cv::imshow("Image With Corners", frame);
  }

  std::cout << "Calibration ended." << std::endl;
  std::cout << calib_frames_count << " frames captured to perform calibration." << std::endl;

  // When everything done, release the video capture object
  cap.release();

  // Closes all the frames
  cv::destroyAllWindows();

  // Get the calibration matrices
  cv::Mat cameraMatrix, distCoeffs, R, T;

  if (calib_frames_count > 0)
  {
    /*
     * Performing camera calibration by
     * passing the value of known 3D points (objpoints)
     * and corresponding pixel coordinates of the
     * detected corners (imgpoints)
     */
    cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

    std::cout << "=====================" << std::endl;
    std::cout << " Calibration Results " << std::endl;
    std::cout << "=====================" << std::endl;
    std::cout << "\ncameraMatrix :\n"
              << cameraMatrix << std::endl;
    std::cout << "\ndistCoeffs :\n"
              << distCoeffs << std::endl;
    std::cout << "\nRotation vector :\n"
              << R << std::endl;
    std::cout << "\nTranslation vector :\n"
              << T << std::endl;
  }

  // Show the image corrected
  switch (input_type)
  {
  case 1:
    std::cout << "Opening video stream." << std::endl;
    cap.open(0);
    if (!cap.isOpened())
    {
      std::cout << "Error opening video stream." << std::endl;
      return -1;
    }
    break;
  case 2:
    std::cout << "Opening video file." << std::endl;
    cap.open(media_path);
    // Check if camera opened successfully
    if (!cap.isOpened())
    {
      std::cout << "Error opening video file." << std::endl;
      return -1;
    }
    break;
  }

  /*while (1)
  {
    // Capture frame-by-frame
    cap >> frame;

    // If the frame is empty, break immediately
    if (frame.empty())
      break;

    cv::Mat newCameraMatrix;
    newCameraMatrix = cv::getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, frame.size(), 1, frame.size());

    // Display the resulting frame
    cv::imshow("Calibrated Image", frame);

    // Press ESC on keyboard to exit,
    cv::waitKey(25);
  }*/
  return 0;

quit:
  // When everything done, release the video capture object
  cap.release();
  // Closes all the frames
  cv::destroyAllWindows();

  return 0;
}

void displaySelectInputType()
{
  std::system("clear");
  std::cout << "=====================" << std::endl;
  std::cout << "  Select Input Type  " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << " 1.Camera" << std::endl;
  std::cout << " 2.Video" << std::endl;
  std::cout << " 3.Images" << std::endl;
  std::cout << " q.Quit" << std::endl;
  std::cout << std::endl;
  std::cout << ">> ";
}

void displaySelectVideoSource()
{
  std::system("clear");
  std::cout << "=====================" << std::endl;
  std::cout << "  Select Video Path  " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << " 1.Default (./Videos/calib.mp4)" << std::endl;
  std::cout << " 2.Custom Path" << std::endl;
  std::cout << std::endl;
  std::cout << ">> ";
}

void displayInsertCustomVideoSource()
{
  std::system("clear");
  std::cout << "=====================" << std::endl;
  std::cout << "  Insert Video Path  " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << std::endl;
  std::cout << ">> ";
}

void displaySelectChessboardDimensions()
{
  std::system("clear");
  std::cout << "=====================" << std::endl;
  std::cout << "     Matrix Size    " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << " 1.Default (H:8 x W:11)" << std::endl;
  std::cout << " 2.Custom Size" << std::endl;
  std::cout << std::endl;
  std::cout << ">> ";
}

void displayChessboardDimensions()
{
  std::system("clear");
  std::cout << "=====================" << std::endl;
  std::cout << " Custom Matrix Size  " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << "Sizes must be integers" << std::endl;
  std::cout << "and greater than 2.  " << std::endl;
  std::cout << std::endl;
}

void displayInsertChessboardHeight()
{
  std::cout << "Height: ";
}

void displayInsertChessboardWidth()
{
  std::cout << "Width: ";
}

void displayLowFrameCountWarning()
{
  std::cout << "=====================" << std::endl;
  std::cout << "      WARNING        " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << "The number of frames " << std::endl;
  std::cout << "captured is low!" << std::endl;
  std::cout << "You should capture " << std::endl;
  std::cout << "some more in order to" << std::endl;
  std::cout << "get better calibration" << std::endl;
  std::cout << "values!" << std::endl;
  std::cout << std::endl;
  std::cout << "Exit anyway?" << std::endl;
  std::cout << "1.No" << std::endl;
  std::cout << "2.Yes" << std::endl;
  std::cout << std::endl;
}

void displayLowFrameCountError()
{
  std::cout << "=====================" << std::endl;
  std::cout << "        ERROR        " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << "The number of frames " << std::endl;
  std::cout << "captured isn't enough!" << std::endl;
  std::cout << "You must capture " << std::endl;
  std::cout << "at least 10 frames to" << std::endl;
  std::cout << "get the calibration" << std::endl;
  std::cout << "values!" << std::endl;
  std::cout << std::endl;
  std::cout << "Do you want to quit?" << std::endl;
  std::cout << "1.No" << std::endl;
  std::cout << "2.Yes" << std::endl;
  std::cout << std::endl;
}

void displaySelectCapturingMode()
{
  std::system("clear");
  std::cout << "=====================" << std::endl;
  std::cout << "   Capturing Mode    " << std::endl;
  std::cout << "=====================" << std::endl;
  std::cout << "1.Manual (by pressing 'c')" << std::endl;
  std::cout << "2.Auto (captures at 0.5Hz)" << std::endl;
  std::cout << std::endl;
  std::cout << ">> ";
}