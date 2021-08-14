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
#include "KeyCombination.h"
#include <unordered_map>
#include <LLUtils/Exception.h>

namespace LInput
{
    template <class BindingType>
    class KeyBindings
    {
    private:
        using MapCombinationToBinding = std::unordered_map<KeyCombination, BindingType, KeyCombination::Hash>;

    public:
      void AddBinding(KeyCombination combination,const BindingType& binding)
      {
          if (static_cast<KeyCode>(combination.keydata().keycode) == KeyCode::UNASSIGNED)
              LL_EXCEPTION(LLUtils::Exception::ErrorCode::LogicError , "trying to add an 'Unassigned' key binding");


          auto ib = mBindings.emplace(combination, binding);
          if (ib.second == false)
              LL_EXCEPTION(LLUtils::Exception::ErrorCode::DuplicateItem, "duplicate entries are not allowed");
      }

      void AddBinding(ListKeyCombinations combination, const BindingType& binding)
      {
          for (const KeyCombination& comb : combination)
              AddBinding(comb, binding);
      }

      bool GetBinding(KeyCombination combination, BindingType& bindingType)
      {
          static BindingType empty;
          typename MapCombinationToBinding::const_iterator it = mBindings.find(combination);
          if (it != mBindings.end())
          {
              bindingType = it->second;
              return true;
          }
          return false;
      }

    private:
        MapCombinationToBinding mBindings;

    };
}
