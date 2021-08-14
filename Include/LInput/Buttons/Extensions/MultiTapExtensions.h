/*
Copyright (c) 2021 Lior Lahav

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

	template <typename button_type>
	class MultitapExtension final : public IButtonStateExtension<button_type>
	{
	public:

		MultitapExtension(uint16_t id, uint16_t multipressRate, uint16_t maxTaps) :
			fID(id)
			, fMultiPressThreshold(multipressRate)
			, fMaxTaps(maxTaps)
		{
			fTimer.SetDueTime(multipressRate);
			fTimer.SetRepeatInterval(INFINITE);
		}

		struct MultiTapEvent
		{
			/// <summary>
			/// Number of taps in the event 1 for tap, 2 for double tap, and so on... 
			/// </summary>
			MultitapExtension* parent;
			button_type button;
			uint16_t tapCount;
		};

		LLUtils::Event<void(const MultiTapEvent&)> OnButtonEvent;
		using ListButtonEvent = std::vector<MultiTapEvent>;
	
		struct ButtonData
		{
			uint64_t timestampLastButtonDown = 0;
			uint16_t tapCounter = 0;
			ButtonState buttonState = ButtonState::Up;
		};


		ButtonData& GetButtonData(button_type buttonId)
		{
			auto it = fMapButttons.find(buttonId);
			if (it == fMapButttons.end())
				it = fMapButttons.emplace(buttonId, ButtonData{}).first;

			return it->second;
		}


		void ProcessQueuedButtons()
		{

			std::set<button_type> buttonsRemoved;
	
			int64_t minTimeToEvent = (std::numeric_limits<int64_t>::min)();
			for (auto& button : fPressedButtons)
			{
				auto& buttonData = GetButtonData(button);
				uint64_t now = static_cast<uint64_t>(fStopWatch.GetElapsedTimeInteger(LLUtils::StopWatch::Milliseconds));

				int64_t timeSinceActuation = static_cast<int64_t>(now) - static_cast<int64_t>( buttonData.timestampLastButtonDown);

				int64_t timeToEvent = timeSinceActuation - fMultiPressThreshold;
				if (timeToEvent >= 0)
				{ 
					OnButtonEvent.Raise(MultiTapEvent{ this,button, buttonData.tapCounter });
					buttonData.tapCounter = 0;
					buttonsRemoved.insert(button);

				}
				else
				{
					
					// timeToEvent is negative, get the closest number to zero.
					minTimeToEvent = (std::max)(minTimeToEvent, timeToEvent);
				}
			}

			if (minTimeToEvent != (std::numeric_limits<int64_t>::min)())
			{
				fTimer.SetDueTime(static_cast<DWORD>( -minTimeToEvent));
				fTimer.Enable(true);
			}

			for (const auto& remove : buttonsRemoved)
				fPressedButtons.erase(remove);

		}

		void TimerCallback()
		{
			ProcessQueuedButtons();
		}

	public:

		uint16_t GetID() const { return fID; }
		// Get the state of a button whether it's down or up
		void SetButtonState(button_type button, ButtonState newState) override
		{
			ButtonData& buttonData = GetButtonData(button);
			const uint64_t currentTimeStamp = static_cast<uint64_t>(fStopWatch.GetElapsedTimeInteger(LLUtils::StopWatch::Milliseconds));

			if (buttonData.buttonState != newState)
			{

				if (newState == ButtonState::Down)
				{

					if (buttonData.buttonState == ButtonState::Up)
					{
						buttonData.tapCounter++;
						buttonData.timestampLastButtonDown = currentTimeStamp;

						if (buttonData.tapCounter < fMaxTaps)
						{
							fPressedButtons.insert(button);
							fTimer.SetDueTime(fMultiPressThreshold);
							fTimer.Enable(true);
						}
						else if (buttonData.tapCounter == fMaxTaps) // reached max taps, raise an event
						{
							fPressedButtons.erase(button);
							
							OnButtonEvent.Raise(MultiTapEvent{ this,button, buttonData.tapCounter });
							buttonData.tapCounter = 0;
							if (fPressedButtons.empty() == true)
							{
								fTimer.Enable(false);
							}
						}
					}
				}

		
				buttonData.buttonState = newState;
			}

		}
	private:

	
		uint16_t fID = 0;
		/// <summary>
		/// time in millisecond between actuation that determines the time threshold between presses
		/// </summary>
		uint16_t fMultiPressThreshold = 250;
		/// <summary>
		/// the repeat rate in milliseconds, set to zero (0) to disable repeat rate
		/// </summary>
		uint16_t fMaxTaps = 3;
	
		::Win32::HighPrecisionTimer fTimer = ::Win32::HighPrecisionTimer(std::bind(&MultitapExtension::TimerCallback, this));

		using MapButtonToData = std::map<button_type, ButtonData>;
		MapButtonToData fMapButttons;
		/// <summary>
		/// used for sending key repeaet signals to the client
		/// </summary>
		std::set<button_type> fPressedButtons;
		LLUtils::StopWatch fStopWatch = LLUtils::StopWatch(true);
	};
}