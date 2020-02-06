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
#include <LInput/Buttons/ButtonType.h>
namespace LInput
{
	
	//template <typename T>
	//	struct MaxButtons
	//{
	//	T value = (1 << (sizeof(T) * 8 - 1)) | (sizeof(T) * 8 - 1);
	//};
	
		template <typename T>
		constexpr size_t MaxValue = (1 << (sizeof(T) * 8 - 1)) | (sizeof(T) * 8 - 1);


		//size_t maxBUttons = Max_Buttons<uint16_t>;
	
		template <typename T>
	class IButtonable
	{
	public:
		virtual void SetButtonState(T button, State  oldstate, State newState) = 0;
	};


	template <typename T>
	class IButtonState
	{
	public:
		virtual void SetButtonState(T button, State state) = 0;
		virtual State GetButtonState(T button) const = 0;
	};


}