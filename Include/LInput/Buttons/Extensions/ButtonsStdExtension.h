/*
Copyright (c) 2020 Lior Lahav

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <LLUtils/StopWatch.h>
#include <LLUtils/Event.h>
#include <LInput/Buttons/ButtonState.h>
#include <LInput/Buttons/IButtonStateExtension.h>
#include <Win32/HighPrecisionTimer.h>


namespace LInput
{

	enum class EventType { NotSet, Pressed, Released };
	
	template <typename button_type>
	class ButtonStdExtension final : public IButtonStateExtension<button_type>
	{

	
	public:
		
		ButtonStdExtension(uint8_t id, uint16_t multipressRate, uint16_t repeatRate) :
		  fID(id)
		, fMultiPressRate(multipressRate)
		, fRepeatRate(repeatRate)
		
		{
			timer.SetDueTime(fRepeatRate);
			timer.SetRepeatInterval(fRepeatRate);
		}


		///////////////////////
		// Button event            
		struct ButtonEvent
		{
			ButtonStdExtension* parent;
			uint64_t timeStamp;
			button_type button;
			EventType eventType;
			/// <summary>
			/// Press count - how many times the button has been pressed, e.g. down + up = 1 press
			/// </summary>
			uint16_t counter;
			/// <summary>
			/// how many times a signal has been sent to client that the button is pressed.
			/// </summary>
			uint16_t repeatCount;
			/// <summary>
			/// Total actuation time
			/// </summary>
			uint16_t actuationTime;
		};

		LLUtils::Event<void(const ButtonEvent&)> OnButtonEvent;

		typedef std::vector<ButtonEvent> ListButtonEvent;
		///////////////////////

		struct ButtonData
		{
			uint64_t timeStamp = 0;
			ButtonState buttonState = ButtonState::Up;
			uint16_t pressCounter = 0;
			uint64_t actuationTimeStamp = 0;
			uint64_t repeatTimeStamp = 0;
			uint16_t repeatCount = 0;
		};




		ButtonData& GetButtonData(button_type buttonId)
		{
			auto it = mMapButttons.find(buttonId);
			if (it == mMapButttons.end())
				it = mMapButttons.emplace(buttonId, ButtonData {}).first;

			return it->second;
		}


		void ProcessQueuedButtons()
		{
			for (auto& button : fPressedButtons)
			{
				auto& buttonData = GetButtonData(button);
				uint64_t now = static_cast<uint64_t>(fTimer.GetElapsedTimeInteger(LLUtils::StopWatch::Milliseconds));

				if (static_cast<uint64_t>(now) - buttonData.repeatTimeStamp > fRepeatRate)
				{
					buttonData.repeatCount++;
					OnButtonEvent.Raise(ButtonEvent{ this, 0,button,EventType::Pressed,buttonData.pressCounter, buttonData.repeatCount , static_cast<uint16_t>(now - buttonData.actuationTimeStamp) });
					buttonData.repeatTimeStamp = now;
				}
			}
		}

		void TimerCallback()
		{
			ProcessQueuedButtons();
		}

	public:
		void SetRepeatRate(uint16_t repeatRate)
		{
			fRepeatRate = repeatRate;
		}

		uint16_t GetRepeatRate() const
		{
			return fRepeatRate;
		}


	
	
		uint8_t GetID() const { return fID; }
		// Get the state of a button whether it's down or up
		 void SetButtonState(button_type button, ButtonState newState) override
		{
			ButtonData& buttonData = GetButtonData(button);
			const uint64_t currentTimeStamp = static_cast<uint64_t>(fTimer.GetElapsedTimeInteger(LLUtils::StopWatch::Milliseconds));
			const bool multiPressTHreshold = buttonData.timeStamp != 0 && (currentTimeStamp - buttonData.timeStamp) < fMultiPressRate;

			if (buttonData.buttonState != newState)
			{

				if (newState == ButtonState::Down)
				{
					if (buttonData.buttonState == ButtonState::Up)
					{
						if (multiPressTHreshold == true)
							buttonData.pressCounter++;
						else
							buttonData.pressCounter = 0;

						if (fRepeatRate > 0)
						{
							fPressedButtons.insert(button);
							buttonData.actuationTimeStamp = currentTimeStamp;
							buttonData.repeatTimeStamp = currentTimeStamp;
							timer.Enable(true);
						}

						OnButtonEvent.Raise(ButtonEvent{this,0 ,button,EventType::Pressed,buttonData.pressCounter, buttonData.repeatCount ,0 });

					}
				}
				if (newState == ButtonState::Up)
				{
					if (fRepeatRate > 0)
					{
						fPressedButtons.erase(button);
						if (fPressedButtons.empty() == true)
							timer.Enable(false);
						
						buttonData.actuationTimeStamp = 0;
						buttonData.repeatTimeStamp = 0;
						buttonData.repeatCount = 0;
					}
					
					OnButtonEvent.Raise(ButtonEvent{this, 0,button,EventType::Released,buttonData.pressCounter, buttonData.repeatCount ,0});
					
					if (multiPressTHreshold == false)
						buttonData.pressCounter = 0;

				}

				buttonData.timeStamp = currentTimeStamp;
				buttonData.buttonState = newState;
			}

		}
	private:
		
		uint8_t fID = 0;
		/// <summary>
		/// time in millisecond between actuation that determines the time threshold between presses
		/// </summary>
		uint16_t fMultiPressRate = 250;
		/// <summary>
		/// the repeat rate in milliseconds, set to zero (0) to disable repeat rate
		/// </summary>
		uint16_t fRepeatRate = 15;
		::Win32::HighPrecisionTimer timer = ::Win32::HighPrecisionTimer(std::bind(&ButtonStdExtension::TimerCallback, this));
		LLUtils::StopWatch fTimer = LLUtils::StopWatch(true);
		using MapButtonToData = std::map<button_type, ButtonData>;
		MapButtonToData mMapButttons;
		/// <summary>
		/// used for sending key repeaet signals to the client
		/// </summary>
		std::set<button_type> fPressedButtons;
	};
}