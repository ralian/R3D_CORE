class R3D_Math
{
	const float PIPow2 = Math.PI * Math.PI;
	const float PIPow3 = Math.PI * Math.PI * Math.PI;
	
	// Absolute entropy in W / (Hz K)
	const float Kb = 1.38e-23;
	
	static vector Multiply(vector data, vector other)
	{
		vector n = vector.Zero;

		n[0] = data[0] * other[0];
		n[1] = data[1] * other[1];
		n[2] = data[2] * other[2];

		return n;
	}
	
	static float Interpolate(float in, float Xmin, float Xmax, float Ymin, float Ymax)
	{
		if (in <= Xmin)
			return Ymin;
			
		if (in >= Xmax)
			return Ymax;
			
		return ((Ymin * (Xmax - in) + Ymax * (in - Xmin)) / (Xmax - Xmin));		
	}
	
	static float Log2(float x, float tolerance = 1e-13)
	{
		if (!x)
		{
			Debug.Error("Math domain error");
			return 0.0;  //! We need to return something, unfortunately EnForce doesn't have a NaN value
		}

		//! Shortcut for x == 1.0
		if (x == 1.0)
			return 0.0;

		//! Shortcut for x == 0.5
		if (x == 0.5)
			return -1.0;

		float log2x = 0.0;

		//! Integer part
		while (x < 1.0)
		{
			log2x -= 1.0;
			x *= 2.0;
		}
		while (x >= 2.0)
		{
			log2x += 1.0;
			x /= 2.0;
		}

		//! Fractional part
		float frac = 1.0;
		while (frac >= tolerance)
		{
			frac /= 2.0;
			x *= x;
			if (x >= 2.0)
			{
				x /= 2.0;
				log2x += frac;
			}
		}

		return log2x;
	}

	static float Log10(float x, float tolerance = 1e-13)
	{
		static const float Log2_10 = Log2(10);
		return Log2(x) / Log2_10;
	}
};
