// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#include <frc2/command/StartEndCommand.h>
#include <frc2/command/InstantCommand.h>
#include <frc2/command/button/Button.h>
#include <frc/Relay.h>
#include <cameraserver/CameraServer.h>
#include "RobotContainer.h"
#include "Constants.h"

RobotContainer::RobotContainer() : m_autoDrive(&m_driveTrain, &m_launcher, &m_intake) {
  // ANOTHER WAY OF CONSTRUCTING: m_autoDrive = AutoDrive(&m_driveTrain);
  // Initialize all of your commands and subsystems here

  // Configure the button bindings
  ConfigureButtonBindings();

#if defined ENABLE_DRIVETRAIN && defined ENABLE_LAUNCHER
// #define ENABLE_FLIGHTSTICK
#ifdef ENABLE_FLIGHTSTICK
  // Set up default drive command
  m_driveTrain.SetDefaultCommand(TeleOpDrive(
    &m_driveTrain,
    [this] { return driver_control.GetRawAxis(ConFlightControl::ELEVATOR); },
    [this] { return driver_control.GetRawAxis(ConFlightControl::RUDDER); }));

#else // !ENABLE_FLIGHTSTICK
  // Set up default drive command
  m_driveTrain.SetDefaultCommand(TeleOpDrive(
    &m_driveTrain,
    [this] { return driver_control.GetRawAxis(ConXBOXControl::RIGHT_TRIGGER) - driver_control.GetRawAxis(ConXBOXControl::LEFT_TRIGGER); },
    // Scale turning by a 25% for at-home challenges  
    // To swap front & back of robot, add/remove a minus sign in the following line, and swap the INVERTED/NONINVERTED in DriveTrain.cpp
    [this] { return -driver_control.GetRawAxis(ConXBOXControl::LEFT_JOYSTICK_X/4); }));
#endif
#endif // ENABLE_DRIVETRAIN

}

void RobotContainer::RobotInit() {

  #ifdef ENABLE_USB_CAMERA
  // Start the Camera Server
  // Get the USB camera from CameraServer
  cs::UsbCamera camera = frc::CameraServer::StartAutomaticCapture();
  // Set the resolution
  camera.SetResolution(640, 480);
  camera.SetFPS(15);
  #endif
  // Create and get reference to SB tab
  m_sbt_Robot = &frc::Shuffleboard::GetTab(ConShuffleboard::RobotTab);
  // See https://docs.wpilib.org/en/latest/docs/software/wpilib-tools/shuffleboard/layouts-with-code/using-tabs.html

  // Create widget for code version
  #define CODE_VERSION ROBOT_VERSION_STRING " " __DATE__ " " __TIME__ 
  m_nte_CodeVersion = m_sbt_Robot->Add("Code Version", CODE_VERSION).WithSize(3, 1).WithPosition(0, 0).GetEntry();

}
// Called ONCE when the robot is disabled
void RobotContainer::DisabledInit() {
  m_driveTrain.ResetEncoders();
  m_launcher.Retract();
  m_intake.Stow();
}

// Called periodically while the robot is disabled
void RobotContainer::DisabledPeriodic() {

}

void RobotContainer::TeleopInit() {
  m_launcher.SetLaunchSoftLimits();
  m_climber.SetClimberSoftLimits();
}

void RobotContainer::ConfigureButtonBindings() {
  // Configure your button bindings here
#ifdef ENABLE_DRIVETRAIN
  // Commence reduced speed driving when bumper(s) pressed
  // frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::RIGHT_BUMPER); }).WhenHeld(new TeleOpSlowDrive(&m_driveTrain));
  // frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::LEFT_BUMPER); }).WhenHeld(new TeleOpSlowDrive(&m_driveTrain));
#endif // ENABLE_DRIVETRAIN

#ifdef ENABLE_LAUNCHER
  // Duplicate Launch OI controls on both driver & codriver inputs
  frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::LEFT_BUMPER); }).WhileHeld(
      frc2::StartEndCommand( [&] {m_launcher.LaunchBert();}, [&] {m_launcher.RetractBert();}, {&m_launcher} ));
  frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::RIGHT_BUMPER); }).WhileHeld(
      frc2::StartEndCommand( [&] {m_launcher.LaunchErnie();}, [&] {m_launcher.RetractErnie();}, {&m_launcher} ));

  frc2::Button([this] { return codriver_control.GetRawButton(ConXBOXControl::LEFT_BUMPER); }).WhileHeld(
      frc2::StartEndCommand( [&] {m_launcher.LaunchBert();}, [&] {m_launcher.RetractBert();}, {&m_launcher} ));
  frc2::Button([this] { return codriver_control.GetRawButton(ConXBOXControl::RIGHT_BUMPER); }).WhileHeld(
      frc2::StartEndCommand( [&] {m_launcher.LaunchErnie();}, [&] {m_launcher.RetractErnie();}, {&m_launcher} ));

#endif

#ifdef ENABLE_INTAKE
  // Duplicate Intake OI controls on both driver & codriver inputs
  frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::A); }).ToggleWhenPressed(
      frc2::StartEndCommand( [&] {m_intake.Deploy();}, [&] {m_intake.Stow();}, {&m_intake} ));
  frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::B); }).WhileHeld(
    frc2::StartEndCommand( [&] {m_intake.Reject(); }, [&] {m_intake.Load();} ));

  frc2::Button([this] { return codriver_control.GetRawButton(ConXBOXControl::A); }).ToggleWhenPressed(
      frc2::StartEndCommand( [&] {m_intake.Deploy();}, [&] {m_intake.Stow();}, {&m_intake} ));
  frc2::Button([this] { return codriver_control.GetRawButton(ConXBOXControl::B); }).WhileHeld(
    frc2::StartEndCommand( [&] {m_intake.Reject(); }, [&] {m_intake.Load();} ));
#endif

#ifdef ENABLE_CLIMBER
  frc2::Button([this] {return codriver_control.GetRawButton(ConXBOXControl::X); }).WhileHeld(new Climb(&m_climber));
  frc2::Button([this] {return codriver_control.GetRawButton(ConXBOXControl::Y); }).WhileHeld(new ExtendClimber(&m_climber));
#endif // ENABLE_CLIMBER

#ifdef ENABLE_TESTING
// Servo Test
  frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::X); }).WhenPressed(Test(&m_testing, 1));
  frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::Y); }).WhenPressed(Test(&m_testing, 0));
#endif

#ifdef ENABLE_VISION
  frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::SELECT); }).WhenPressed(
    frc2::InstantCommand( [&] {m_vision.ToggleLight();}, { &m_vision } ));
  //frc2::Button([this] { return driver_control.GetRawButton(ConXBOXControl::START); }).WhenPressed(Test(&m_testing, 0));
#endif
}

frc2::Command* RobotContainer::GetAutonomousCommand() {
  // An example command will be run in autonomous
  return &m_autoDrive;
}
