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
#include <LInput/Buttons/ButtonState.h>
#include <LInput/Buttons/IButtonStateExtension.h>
#include <LLUtils/Event.h>

namespace LInput
{

	template <typename button_type>
	class IButtonState
	{
	public:
		virtual void SetButtonState(button_type button, ButtonState state) = 0;
		virtual ButtonState GetButtonState(button_type button) const = 0;
		virtual ~IButtonState() = default;
	};

	template <typename button_type, size_t NUM_BUTTONS = MaxValue<button_type>>
	class ButtonsState  : public IButtonState<button_type> 
	{
	public:
		using ExtensionType = std::shared_ptr<IButtonStateExtension<button_type>>;
		using VecExtensionsType = std::vector<ExtensionType>;
		using underlying_button_type = button_type;
	

	public:

		ButtonsState()
		{
			fButtonStates.fill(ButtonState::Up);
		}

		ButtonState GetButtonState(button_type  buttonId) const override
		{
			return fButtonStates[static_cast<size_t>( buttonId)];

		}

		// Get the state of a button whether it's down or up
		void SetButtonState(button_type button, ButtonState newState) override
		{
			
			ButtonState oldState = fButtonStates[static_cast<size_t>(button)];
			
			if (oldState != newState && newState != ButtonState::NotSet)
			{
				fButtonStates[static_cast<size_t>(button)] = newState;
				for (ExtensionType& e : fButtonExtensions)
					e->SetButtonState(button, newState);

			}
		}

		void AddExtension(ExtensionType extension)
		{
			fButtonExtensions.push_back(extension);
		}
	private:
		VecExtensionsType fButtonExtensions;
		std::array<ButtonState, NUM_BUTTONS> fButtonStates;

	};
}