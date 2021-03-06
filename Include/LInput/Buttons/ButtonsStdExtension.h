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
#include <LInput/Buttons/ButtonType.h>
#include <LInput/Buttons/IButtonable.h>
#include <Win32/HighPrecisionTimer.h>


namespace LInput
{

	enum class EventType { NotSet, Pressed, Released };
	
	template <typename T>
	class ButtonStdExtension final : public IButtonable<T>
	{
	
	public:
		using ButtonType = T;
		
		ButtonStdExtension(uint16_t id, uint16_t multipressRate, uint16_t repeatRate) :
		  fID(id)
		, fMultiPressRate(multipressRate)
		, fRepeatRate(repeatRate)
		{
			timer.SetDelay(fRepeatRate);
		}


		///////////////////////
		// Button event            
		struct ButtonEvent
		{
			ButtonStdExtension* parent;
			uint64_t timeStamp;
			ButtonType button;
			EventType eventType;
			uint16_t counter;
			uint16_t repeatCount;
			uint16_t actuationTime;

		};

		LLUtils::Event<void(const ButtonEvent&)> OnButtonEvent;

		typedef std::vector<ButtonEvent> ListButtonEvent;
		///////////////////////

		struct ButtonData
		{
			uint64_t timeStamp = 0;
			State buttonState = State::Up;
			uint16_t pressCounter = 0;
			uint64_t actuationTimeStamp = 0;
			uint64_t repeatTimeStamp = 0;
			uint16_t repeatCount = 0;
		};




		ButtonData& GetButtonData(ButtonType buttonId)
		{
			auto it = mMapButttons.find(buttonId);
			if (it == mMapButttons.end())
				it = mMapButttons.emplace(static_cast<uint16_t>(buttonId), ButtonData{}).first;

			return it->second;
		}



	public:
		void ProcessQueuedButtons()
		{
			for (auto& button : fPressedButtons)
			{
				auto& buttonData = GetButtonData(button);
				uint64_t now = static_cast<uint64_t>(fTimer.GetElapsedTimeInteger(LLUtils::StopWatch::Milliseconds));

				if ( static_cast<uint64_t>(now) - buttonData.repeatTimeStamp > fRepeatRate)
				{
					buttonData.repeatCount++;
					OnButtonEvent.Raise(ButtonEvent{ this, 0,button,EventType::Pressed,buttonData.pressCounter, buttonData.repeatCount , static_cast<uint16_t>(now - buttonData.actuationTimeStamp)});
					buttonData.repeatTimeStamp = now;
				}
			}
		}
		
		void TimerCallback()
		{
			ProcessQueuedButtons();
		}
		
		
		uint16_t GetID() const { return fID; }
		// Get the state of a button whether it's down or up
		void SetButtonState(ButtonType button, State newState) override
		{
			ButtonData& buttonData = GetButtonData(button);
			uint64_t currentTimeStamp = static_cast<uint64_t>(fTimer.GetElapsedTimeInteger(LLUtils::StopWatch::Milliseconds));
			bool multiPressTHreshold = buttonData.timeStamp != 0 && (currentTimeStamp - buttonData.timeStamp) < fMultiPressRate;

			if (buttonData.buttonState != newState)
			{

				if (newState == State::Down)
				{
					if (buttonData.buttonState == State::Up)
					{
						if (multiPressTHreshold == true)
							buttonData.pressCounter++;
						else
							buttonData.pressCounter = 0;

						if (fEnableRepeat == true)
						{
							fPressedButtons.insert(button);
							buttonData.actuationTimeStamp = currentTimeStamp;
							buttonData.repeatTimeStamp = currentTimeStamp;
							timer.Enable(true);
						}

						OnButtonEvent.Raise(ButtonEvent{this, 0,button,EventType::Pressed,buttonData.pressCounter, buttonData.repeatCount ,static_cast<uint16_t>(currentTimeStamp - buttonData.actuationTimeStamp) });

					}
				}
				if (newState == State::Up)
				{
					if (fEnableRepeat == true)
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
		uint16_t fID = 0;
		uint16_t fMultiPressRate = 250;
		uint16_t fRepeatRate = 15;
		::Win32::HighPrecisionTimer timer = ::Win32::HighPrecisionTimer(std::bind(&ButtonStdExtension::TimerCallback, this));
		LLUtils::StopWatch fTimer = LLUtils::StopWatch(true);
		using MapButtonToData = std::map<uint16_t, ButtonData>;
		MapButtonToData mMapButttons;
		std::set<ButtonType> fPressedButtons;
		bool fEnableRepeat = true;
	};
}