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
#include <LLUtils/StopWatch.h>
#include <LLUtils/Event.h>
#include <LInput/Buttons/ButtonType.h>
#include <LInput/Buttons/IButtonable.h>


namespace LInput
{
	template <typename T>
	class ButtonStdExtension final : public IButtonable<T>
	{
	
	public:
		using ButtonType = T;
		enum class EventType { NotSet, Pressed, Released };


		///////////////////////
		// Button event            
		struct ButtonEvent
		{
			uint64_t timeStamp;
			ButtonType button;
			EventType eventType;
			uint16_t counter;

		};

		LLUtils::Event<void(const ButtonEvent&)> OnButtonEvent;

		typedef std::vector<ButtonEvent> ListButtonEvent;
		///////////////////////

		struct ButtonData
		{
			uint64_t timeStamp;
			State buttonState;
			uint16_t pressCounter;
		};




		ButtonData& GetButtonData(ButtonType buttonId)
		{
			auto it = mMapButttons.find(buttonId);
			if (it == mMapButttons.end())
				it = mMapButttons.insert(std::make_pair(buttonId, ButtonData{ 0u, State::Up, 0 })).first;

			return it->second;
		}



	public:

		// Get the state of a button whether it's down or up
		void SetButtonState(ButtonType button, State oldstate, State newState) override
		{

			ButtonData& buttonData = GetButtonData(button);
			uint64_t elpased = fTimer.GetElapsedTimeInteger(LLUtils::StopWatch::Milliseconds);
			bool multiPressTHreshold = (elpased - buttonData.timeStamp) < fDoublePressThreshold;

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

						OnButtonEvent.Raise(ButtonEvent{ 0,button,EventType::Pressed,buttonData.pressCounter });

					}
				}
				if (newState == State::Up)
				{
					OnButtonEvent.Raise(ButtonEvent{ 0,button,EventType::Released,buttonData.pressCounter });

					if (multiPressTHreshold == false)
						buttonData.pressCounter = 0;

				}

				buttonData.timeStamp = elpased;
				buttonData.buttonState = newState;
			}

		}
	private:
		LLUtils::StopWatch fTimer = LLUtils::StopWatch(true);
		using MapButtonToData = std::map<uint16_t, ButtonData>;
		MapButtonToData mMapButttons;
		uint16_t fDoublePressThreshold = 250;
	};
}