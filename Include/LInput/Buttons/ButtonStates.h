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
#include <memory>
#include <LLUtils/StopWatch.h>
#include <LInput/Buttons/ButtonType.h>
#include <LInput/Buttons/ButtonsStdExtension.h>
#include <LInput/Buttons/IButtonable.h>
#include <LLUtils/Event.h>

namespace LInput
{
	template <typename T, size_t NUM_BUTTONS = MaxValue<T> >
	class ButtonsState : public IButtonState<T>
	{
	public:
		using underlying_type = T;
		using ButtonType = T;
		using VecButtonExtensions = std::vector<std::shared_ptr<IButtonable<T>>>;

	public:

		State GetButtonState(ButtonType  buttonId) const override
		{
			return fButtonStates[buttonId];

		}

		// Get the state of a button whether it's down or up
		void SetButtonState(ButtonType button, State newState) override
		{
			
			State oldState = fButtonStates[button];
			
			if (oldState != newState && newState != State::NotSet)
			{
				fButtonStates[button] = newState;
				for (std::shared_ptr<IButtonable<T>>& e : fButtonExtensions)
					e->SetButtonState(button, oldState, newState);

			}
		}

		void AddExtension(std::shared_ptr<IButtonable<T>> extension)
		{
			fButtonExtensions.push_back(extension);
		}
	private:
		VecButtonExtensions fButtonExtensions;
		std::array<State, NUM_BUTTONS> fButtonStates;

	};
}