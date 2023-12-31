#pragma once

#include "ER_CoreComponent.h"

namespace EveryRay_Core
{
	class ER_Keyboard : public ER_CoreComponent
	{
		RTTI_DECLARATIONS(ER_Keyboard, ER_CoreComponent)

	public:
		ER_Keyboard(ER_Core& game, LPDIRECTINPUT8 directInput);
		~ER_Keyboard();

		const byte* const CurrentState() const;
		const byte* const LastState() const;

		virtual void Initialize() override;
		virtual void Update(const ER_CoreTime& gameTime) override;

		bool IsKeyUp(byte key) const;
		bool IsKeyDown(byte key) const;
		bool WasKeyUp(byte key) const;
		bool WasKeyDown(byte key) const;
		bool WasKeyPressedThisFrame(byte key) const;
		bool WasKeyReleasedThisFrame(byte key) const;
		bool IsKeyHeldDown(byte key) const;

	private:
		ER_Keyboard();

		static const int KeyCount = 256;

		ER_Keyboard(const ER_Keyboard& rhs);

		LPDIRECTINPUT8 mDirectInput;
		LPDIRECTINPUTDEVICE8 mDevice;
		byte mCurrentState[KeyCount];
		byte mLastState[KeyCount];
	};
}
