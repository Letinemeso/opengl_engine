#ifndef __UTILITY
#define __UTILITY

#include "../Debug.h"

#include "../OpenGL/GLM/vec3.hpp"

#include <utility>


namespace LEti {

namespace Math {

	constexpr float PI = 3.141593f;
	constexpr float HALF_PI = PI / 2.0f;
	constexpr float QUARTER_PI = HALF_PI / 2.0f;
	constexpr float DOUBLE_PI = PI * 2.0f;

	float vector_length(const glm::vec3& _vec);

	void shrink_vector_to_1(glm::vec3& _vec);

	float get_distance(const glm::vec3& _first, const glm::vec3& _second);

    glm::vec3 normalize(const glm::vec3& _first, const glm::vec3& _second);

    float mixed_vector_multiplication(const glm::vec3& _first, const glm::vec3& _second, const glm::vec3& _third);

	bool is_digit(char _c);

    float angle_cos_between_vectors(const glm::vec3& _first, const glm::vec3& _second);

    bool beams_cross_at_right_angle(const glm::vec3& _first, const glm::vec3& _second);

	bool floats_are_equal(float _first, float _second);

}	/*Math*/

namespace Geometry {

	struct Intersection_Data
	{
		enum class Type
		{
			none = 0,
			intersection,
			same_line
		};
		Type type = Type::none;
		glm::vec3 point{0.0f, 0.0f, 0.0f};
		float time_of_intersection_ratio = 1.0f;

		Intersection_Data() { }
		Intersection_Data(Type _type) : type(_type) { }
		Intersection_Data(Type _type, const glm::vec3& _point) : type(_type), point(_point) { }
		Intersection_Data(const Intersection_Data& _other) : type(_other.type), point(_other.point) { }
		Intersection_Data(Intersection_Data&& _other) : type(_other.type), point(_other.point) { }
		void operator=(const Intersection_Data& _other) { type = _other.type; point = _other.point; }
		operator bool() { return type != Type::none; }
	};

}

namespace Geometry_2D {

	class Equasion_Data
	{
	private:
		bool m_is_ok = true;

		bool m_vertical = false;
		float m_x_if_vertical = 0.0f;
		bool m_horisontal = false;

		float m_k = 0.0f, m_b = 0.0f;
		bool m_goes_left = false;
	public:
		Equasion_Data(const glm::vec3& _point_1, const glm::vec3& _point_2);

		bool is_ok() const { return m_is_ok; }
		bool is_vertical() const { return m_vertical; }
		bool is_horisontal() const { return m_horisontal; }
		bool goes_left() const { return m_goes_left; }

		float get_x_if_vertical() const { return m_x_if_vertical; }
		float get_y_if_horisontal() const { return m_b; }

		float get_k() const { return m_k; }
		float get_b() const { return m_b; }

		float solve_by_x(float _x) const { return m_k * _x + m_b; }
		float solve_by_y(float _y) const { return is_horisontal() ? m_b : (_y - m_b) / m_k; }
	};


	Geometry::Intersection_Data lines_intersect(const Equasion_Data& _first, const Equasion_Data& _second);

	Geometry::Intersection_Data segments_intersect(const glm::vec3& _point_11, const glm::vec3& _point_21, const glm::vec3& _point_12, const glm::vec3& _point_22);

}	/*Geometry_2D*/

}	/*LEti*/

#endif
