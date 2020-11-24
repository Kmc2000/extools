class vector{
	public:
		float get_x();
		float get_y();
		vector(float x, float y);
		void set_x(float x);
		void set_y(float y);
		void operator + (float x);
		void operator + (vector& other);
		void operator - (float x);
		void operator - (vector& other);
		bool is_separating_axis(vector& other);

	private:
		float x = 0;
		float y = 0;
};