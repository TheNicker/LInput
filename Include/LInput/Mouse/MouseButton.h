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
#include <vector>
namespace LInput
{

	enum class MouseButton : uint8_t
	{
		  Button1
		, Button2
		, Button3
		, Button4
		, Button5
		, Button6
		, Button7
		, Button8
		, Left		= Button1
		, Right		= Button2
		, Middle	= Button3
		, Back		= Button4
		, Forward	= Button5
		, Count		= Button8 + 1
	};


	const std::vector<const char*> MouseButtonString
	{
		 "MouseButtonLeft"
		,"MouseButtonMiddle"
		,"MouseButtonRight"
		,"MouseButtonBack" 
		,"MouseButtonForward" 
		,"MouseButton_6" 
		,"MouseButton_7" 
		,"MouseButton_8"
	};
	
}