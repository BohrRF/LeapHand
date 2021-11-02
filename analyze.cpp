#include <vector>
#include <chrono>
#include <windows.h>
#include "analyze.h"


//#define OLD

using std::cout;
using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::time_point;

const std::string fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
const std::string boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };
const std::string stateNames[] = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };

void SampleListener::onInit(const Controller& controller) {
    std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
    controller.enableGesture(Gesture::TYPE_CIRCLE);
    controller.enableGesture(Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
    // Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
    system("CLS");
    //std::cout << "Exited" << std::endl;
}
/*
void SampleListener::refreshGraph(const double& x, const double& y)
{
    // Display Frame
    static int lastX = 0, lastY = 0;
    for (int x = 0; x < nScreenWidth; x++)
        screen[nScreenWidth * (nScreenHeight-1) / 2 + x] = 0x2501;
    for (int y = 0; y < nScreenHeight; y++)
        screen[nScreenWidth / 2 + y * nScreenWidth] = 0x2503;

    screen[nScreenWidth * nScreenHeight - 1] = '\0';
    WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
}
*/

#ifdef OLD

void SampleListener::onFrame(const Controller& controller) {
    HandList hands = controller.frame().hands();
    if (GetAsyncKeyState(VK_RETURN) & 0x8000)
    {
        beats = 0;
        startTimePoint = 0;
    }
    if (hands.isEmpty())
        positionSeries.push_back(std::make_pair(0, -1)); //record even no hand is detected
    else
    {
        positionSeries.push_back(std::make_pair(controller.frame().timestamp(), hands.rightmost().palmPosition().y));
        
        if (controller.frame(1).hands().rightmost().palmPosition().y - positionSeries.back().second >= 0)
        {
            if (beats == 0)
                startTimePoint = positionSeries.back().first;
            if (!isLowest)
            {
                ++beats;
                system("CLS");
                printf("bpm: %.1f\n", 60000000 * (beats - 1.0) / static_cast<double>(positionSeries.back().first - startTimePoint));
                isLowest = true;
            }
        }
        else
            isLowest = false;
    }
    
    //showFrameInfo(controller);
}

#endif // DEBUG

#ifndef OLD
const int SPECLEN = 20;
void SampleListener::onFrame(const Controller& controller) {
    const HandList &hands = controller.frame().hands();
    // Get fingers
    const FingerList &fingers = hands.rightmost().fingers();

    //int64_t soundEndTime = 0;
    bool soundOn = false;
    bool ifKeepE = false;
    int64_t curTimeStamp = controller.frame().timestamp();


    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        if (!ifKeepE)
        {
            con.playState = 1;
            ifKeepE = true;
        }
    }
    else
    {
        ifKeepE = false;
    }

    if (GetAsyncKeyState(VK_RETURN) & 0x8000)
    {
        fft.reset();
        con.resetPlayState();
    }
    
    if (hands.isEmpty())
    {
        //cout << "Current Position: NULL" << '\n';
        fft.reset();
        //fft.push(controller.frame().timestamp(), fft.getAverage()); //record even no hand is detected
    }
    else
    {
        float indexFingerX = fingers[1].bone(Bone::TYPE_DISTAL).nextJoint().x;
        float indexFingerY = fingers[1].bone(Bone::TYPE_DISTAL).nextJoint().y;  
        float palmX = hands.rightmost().palmPosition().x;
        float palmY = hands.rightmost().palmPosition().y;
        float wristX = hands.rightmost().arm().wristPosition().x;
        float wristY = hands.rightmost().arm().wristPosition().y;
        
        
        if (hands.rightmost().grabAngle() > 2.9)
        {
            con.playState = 0;
            con.resetBeat();
            fft.reset();
            return;
        }
        else if (con.playState == 0)
        {
            con.playState = 1;
            con.beat_count = 0;
        }
        else
        {
            
        }
            
        fp << curTimeStamp << '\t' 
           << indexFingerX << '\t' << indexFingerY << '\t' 
           << palmX        << '\t' << palmY        << '\t'
           << wristX       << '\t' << wristY;

        fft.push(curTimeStamp, palmX, palmY);
        fingerPosList.push(curTimeStamp, indexFingerX, indexFingerY);

        //cout << "Current Position: " << hands.rightmost().palmPosition().y << '\n';
        //cout << "Current Speed: " << fft.history(0).speed << '\n';

        fft.FFT();
        std::vector<std::pair<double, double>> maxlist;

        fft.find_max_freq(maxlist); // 0 Hz amplitude was eliminated from this list

        //cout << "cur time: " << curTimeStamp << " period: " << 8LL * 1000000 / maxlist[0] << '\n';
        system("cls");
        for (int i = 0; i < SPECLEN; i++)
        {
            printf("%f, %f\n", 60*maxlist[i].first, maxlist[i].second);
        }
          
        //const auto var = fft.getSpeedVariance(static_cast<int64_t>(curTimeStamp - 8LL * 1000000 / maxlist[0]));//length of 8beats
        //cout << "Variance: " << var << '\n';
       

        if (fft.history(1).position.y < fft.history().position.y)
        {
            if (!isLowest)
            {
                if (con.playState == 1)
                {
                    if (fft.freqAvalible())
                    {
                        if (con.beat_count >= con.getMusicInfo().first)
                        {
                            con.playState = 2;
                        }
                        else
                            con.beat_count++;
                    }
                    else
                        con.beat_count = 0;
                }

                //system("cls");



                ++beats;
                //printf("bpm: %.1f\n", 60000000 * (beats - 1.0) / static_cast<double>(positionSeries.back().first - startTimePoint));
                //cout << hand_peak - hands.rightmost().palmPosition().y << endl;
                

                auto accl_temp = fft.calCurAccel(lastPeakTimeStamp, fingerPosList);
                fp << '\t' << accl_temp;

                con.onBeat (curTimeStamp, 60 * maxlist[0].first, 
                            hand_peak - fft.history().position.y,
                            accl_temp);
                isLowest = true;
            }
        }
        else
        {
            isLowest = false;         
        }
        con.refresh(curTimeStamp, fft);

        if (fft.history(1).position.y > fft.history().position.y)
        {
            if (!isHighest)
            {
                hand_peak = fft.history(1).position.y;
                lastPeakTimeStamp = curTimeStamp;
                isHighest = true;
            }
        }
        else
        {
            isHighest = false;
        }
       
        fp << endl;
    }
    

    //refreshGraph(hands.rightmost().palmPosition().x, hands.rightmost().palmPosition().y);
}

#endif // !OLD


void SampleListener::onFocusGained(const Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
    fft.reset();
}

void SampleListener::onFocusLost(const Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
    system("pause");
}

void SampleListener::onDeviceChange(const Controller& controller) {
    std::cout << "Device Changed" << std::endl;
    const DeviceList devices = controller.devices();

    for (int i = 0; i < devices.count(); ++i) {
        std::cout << "id: " << devices[i].toString() << std::endl;
        std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
    }
}

void SampleListener::onServiceConnect(const Controller& controller) {
    std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
    std::cout << "Service Disconnected" << std::endl;
}


void beatAnalyze::showFrameInfo(const Controller& controller)
{
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();
    
    std::cout << "Frame id: " << frame.id()
        << ", timestamp: " << frame.timestamp()
        << ", hands: " << frame.hands().count()
        << ", extended fingers: " << frame.fingers().extended().count()
        << ", tools: " << frame.tools().count()
        << ", gestures: " << frame.gestures().count() << std::endl;
    
    HandList hands = frame.hands();
    for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
        // Get the first hand
        const Hand hand = *hl;
        std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
        std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
            << ", palm position: " << hand.palmPosition() << std::endl;

        // Get the hand's normal vector and direction
        const Vector normal = hand.palmNormal();
        const Vector direction = hand.direction();

        // Calculate the hand's pitch, roll, and yaw angles
        std::cout << std::string(2, ' ') << "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
            << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
            << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;

        // Get the Arm bone
        Arm arm = hand.arm();
        std::cout << std::string(2, ' ') << "Arm direction: " << arm.direction()
            << " wrist position: " << arm.wristPosition()
            << " elbow position: " << arm.elbowPosition() << std::endl;

        // Get fingers
        const FingerList fingers = hand.fingers();
        for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
            const Finger finger = *fl;
            std::cout << std::string(4, ' ') << fingerNames[finger.type()]
                << " finger, id: " << finger.id()
                << ", length: " << finger.length()
                << "mm, width: " << finger.width() << std::endl;

            // Get finger bones
            for (int b = 0; b < 4; ++b) {
                Bone::Type boneType = static_cast<Bone::Type>(b);
                Bone bone = finger.bone(boneType);
                std::cout << std::string(6, ' ') << boneNames[boneType]
                    << " bone, start: " << bone.prevJoint()
                    << ", end: " << bone.nextJoint()
                    << ", direction: " << bone.direction() << std::endl;
            }
        }
    }

    // Get tools
    const ToolList tools = frame.tools();
    for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
        const Tool tool = *tl;
        std::cout << std::string(2, ' ') << "Tool, id: " << tool.id()
            << ", position: " << tool.tipPosition()
            << ", direction: " << tool.direction() << std::endl;
    }

    // Get gestures
    const GestureList gestures = frame.gestures();
    for (int g = 0; g < gestures.count(); ++g) {
        Gesture gesture = gestures[g];

        switch (gesture.type()) {
        case Gesture::TYPE_CIRCLE:
        {
            CircleGesture circle = gesture;
            std::string clockwiseness;

            if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI / 2) {
                clockwiseness = "clockwise";
            }
            else {
                clockwiseness = "counterclockwise";
            }

            // Calculate angle swept since last frame
            float sweptAngle = 0;
            if (circle.state() != Gesture::STATE_START) {
                CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
                sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * Leap::PI;
            }
            std::cout << std::string(2, ' ')
                << "Circle id: " << gesture.id()
                << ", state: " << stateNames[gesture.state()]
                << ", progress: " << circle.progress()
                << ", radius: " << circle.radius()
                << ", angle " << sweptAngle * RAD_TO_DEG
                << ", " << clockwiseness << std::endl;
            break;
        }
        case Gesture::TYPE_SWIPE:
        {
            SwipeGesture swipe = gesture;
            std::cout << std::string(2, ' ')
                << "Swipe id: " << gesture.id()
                << ", state: " << stateNames[gesture.state()]
                << ", direction: " << swipe.direction()
                << ", speed: " << swipe.speed() << std::endl;
            break;
        }
        case Gesture::TYPE_KEY_TAP:
        {
            KeyTapGesture tap = gesture;
            std::cout << std::string(2, ' ')
                << "Key Tap id: " << gesture.id()
                << ", state: " << stateNames[gesture.state()]
                << ", position: " << tap.position()
                << ", direction: " << tap.direction() << std::endl;
            break;
        }
        case Gesture::TYPE_SCREEN_TAP:
        {
            ScreenTapGesture screentap = gesture;
            std::cout << std::string(2, ' ')
                << "Screen Tap id: " << gesture.id()
                << ", state: " << stateNames[gesture.state()]
                << ", position: " << screentap.position()
                << ", direction: " << screentap.direction() << std::endl;
            break;
        }
        default:
            std::cout << std::string(2, ' ') << "Unknown gesture type." << std::endl;
            break;
        }
    }

    if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
        std::cout << std::endl;
    }
}

