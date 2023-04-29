#include <Physics/Physical_Models/Physical_Model_2D.h>
#include <Physics/Physical_Models/Rigid_Body_Polygon.h>
using namespace LEti;


//  Physical_Model_2D

Polygon_Holder_Base* Physical_Model_2D::M_create_polygons_holder() const
{
    Polygon_Holder_Base* holder = new Polygon_Holder<Polygon>;
    return holder;
}



void Physical_Model_2D::M_update_rectangular_border()
{
    L_ASSERT(m_polygons_holder);

    {
        const Polygon& polygon = *m_polygons_holder->get_polygon(0);

        m_current_border.left = polygon[0].x;
        m_current_border.right = polygon[0].x;
        m_current_border.top = polygon[0].y;
        m_current_border.bottom = polygon[0].y;
    }

	for(unsigned int i=0; i<m_polygons_count; ++i)
    {
        const Polygon& polygon = *m_polygons_holder->get_polygon(i);

		for(unsigned int p=0; p<3; ++p)
        {
            if(m_current_border.left > polygon[p].x) m_current_border.left = polygon[p].x;
            if(m_current_border.right < polygon[p].x) m_current_border.right = polygon[p].x;
            if(m_current_border.top < polygon[p].y) m_current_border.top = polygon[p].y;
            if(m_current_border.bottom > polygon[p].y) m_current_border.bottom = polygon[p].y;
		}
	}
}

glm::vec3 Physical_Model_2D::M_calculate_center_of_mass() const
{
    glm::vec3 result(0.0f, 0.0f, 0.0f);

    for(unsigned int i=0; i<get_polygons_count(); ++i)
        result += get_polygon(i)->center();
    result /= (float)get_polygons_count();

    return result;
}



const Geometry_2D::Rectangular_Border& Physical_Model_2D::curr_rect_border() const
{
	return m_current_border;
}



Physical_Model_2D::Physical_Model_2D()
{

}

Physical_Model_2D::Physical_Model_2D(const Physical_Model_2D& _other)
{
	setup(_other.m_raw_coords, _other.m_raw_coords_count, _other.m_collision_permissions);
	copy_real_coordinates(_other);
}

void Physical_Model_2D::setup(const float* _raw_coords, unsigned int _raw_coords_count, const bool* _collision_permissions)
{
	delete[] m_raw_coords;

	m_raw_coords_count = _raw_coords_count;
	m_raw_coords = new float[m_raw_coords_count];
	for (unsigned int i = 0; i < m_raw_coords_count; ++i)
		m_raw_coords[i] = _raw_coords[i];

	delete[] m_collision_permissions;

	m_collision_permissions = new bool[m_raw_coords_count / 3];
	for(unsigned int i=0; i<m_raw_coords_count / 3; ++i)
		m_collision_permissions[i] = _collision_permissions[i];

    delete[] m_polygons_holder;
    m_polygons_holder = M_create_polygons_holder();

    m_polygons_count = m_raw_coords_count / 9;
    m_polygons_holder->allocate(m_polygons_count);
	for (unsigned int i = 0; i < m_polygons_count; ++i)
        m_polygons_holder->get_polygon(i)->setup(&m_raw_coords[i * 9], &m_collision_permissions[i * 3]);
}

void Physical_Model_2D::move_raw(const glm::vec3 &_stride)
{
	for(unsigned int i=0; i<m_raw_coords_count; i += 3)
	{
		m_raw_coords[i] += _stride.x;
		m_raw_coords[i + 1] += _stride.y;
		m_raw_coords[i + 2] += _stride.z;
	}

	for(unsigned int i=0; i < m_polygons_count; ++i)
        m_polygons_holder->get_polygon(i)->calculate_center();
}

Physical_Model_2D::~Physical_Model_2D()
{
	delete[] m_raw_coords;
    delete m_polygons_holder;
}


void Physical_Model_2D::update(const glm::mat4x4& _matrix)
{
    L_ASSERT(m_polygons_holder);

	for (unsigned int i = 0; i < m_polygons_count; ++i)
        m_polygons_holder->get_polygon(i)->update_points_with_single_matrix(_matrix);

	M_update_rectangular_border();
    m_center_of_mass = M_calculate_center_of_mass();
}

void Physical_Model_2D::copy_real_coordinates(const Physical_Model_2D &_other)
{
	for (unsigned int i = 0; i < m_polygons_count; ++i)
	{
		for(unsigned int points_i = 0; points_i < 3; ++points_i)
            m_polygons_holder->get_polygon(i)[points_i] = _other.m_polygons_holder->get_polygon(i)[points_i];
	}
	m_current_border = _other.m_current_border;
}


Physical_Model_2D_Imprint* Physical_Model_2D::create_imprint() const
{
    return new Physical_Model_2D_Imprint(this);
}



const Polygon* Physical_Model_2D::get_polygon(unsigned int _index) const
{
    L_ASSERT(m_polygons_holder && _index < m_polygons_count);

    return m_polygons_holder->get_polygon(_index);
}

const Polygon_Holder_Base* Physical_Model_2D::get_polygons() const
{
    return m_polygons_holder;
}

unsigned int Physical_Model_2D::get_polygons_count() const
{
	return m_polygons_count;
}



//  Physical_Model_2D_Imprint

Physical_Model_2D_Imprint::Physical_Model_2D_Imprint(const Physical_Model_2D* _parent)
{
    m_parent = _parent;
    m_polygons_count = m_parent->get_polygons_count();
    m_polygons_holder = m_parent->get_polygons()->create_copy();
    m_polygons_holder->allocate(m_polygons_count);
    for(unsigned int i=0; i<m_polygons_count; ++i)
        m_polygons_holder->get_polygon(i)->setup(*m_parent->get_polygon(i));
    m_rect_border = m_parent->curr_rect_border();
}


Physical_Model_2D_Imprint::Physical_Model_2D_Imprint(Physical_Model_2D_Imprint&& _other)
{
    m_polygons_holder = _other.m_polygons_holder;
    _other.m_polygons_holder = nullptr;
    m_polygons_count = _other.m_polygons_count;
    _other.m_polygons_count = 0;
    m_parent = _other.m_parent;
    _other.m_parent = nullptr;
    m_rect_border = _other.m_rect_border;
}

Physical_Model_2D_Imprint::Physical_Model_2D_Imprint(const Physical_Model_2D_Imprint& _other)
{
    m_parent = _other.m_parent;
    m_polygons_count = _other.m_polygons_count;
    m_polygons_holder = _other.m_polygons_holder->create_copy();
    m_polygons_holder->allocate(m_polygons_count);
    for(unsigned int i=0; i<m_polygons_count; ++i)
        m_polygons_holder->get_polygon(i)->setup(*_other.m_polygons_holder->get_polygon(i));
    m_rect_border = _other.m_rect_border;
}

Physical_Model_2D_Imprint::~Physical_Model_2D_Imprint()
{
    delete m_polygons_holder;
}



void Physical_Model_2D_Imprint::M_update_rectangular_border()
{
    L_ASSERT(m_polygons_holder);

    {
        const Polygon& polygon = *m_polygons_holder->get_polygon(0);

        m_rect_border.left = polygon[0].x;
        m_rect_border.right = polygon[0].x;
        m_rect_border.top = polygon[0].y;
        m_rect_border.bottom = polygon[0].y;
    }

    for(unsigned int i=0; i<m_polygons_count; ++i)
    {
        const Polygon& polygon = *m_polygons_holder->get_polygon(i);

        for(unsigned int p=0; p<3; ++p)
        {
            if(m_rect_border.left > polygon[p].x) m_rect_border.left = polygon[p].x;
            if(m_rect_border.right < polygon[p].x) m_rect_border.right = polygon[p].x;
            if(m_rect_border.top < polygon[p].y) m_rect_border.top = polygon[p].y;
            if(m_rect_border.bottom > polygon[p].y) m_rect_border.bottom = polygon[p].y;
        }
    }
}



void Physical_Model_2D_Imprint::update(const glm::mat4x4 &_translation, const glm::mat4x4 &_rotation, const glm::mat4x4 &_scale)
{
    L_ASSERT(m_polygons_holder);

    glm::mat4x4 result_matrix = _translation * _rotation * _scale;

    for (unsigned int i = 0; i < m_polygons_count; ++i)
        m_polygons_holder->get_polygon(i)->update_points_with_single_matrix(result_matrix);
}

void Physical_Model_2D_Imprint::update_with_single_matrix(const glm::mat4x4& _matrix)
{
    L_ASSERT(m_polygons_holder);

    for(unsigned int i=0; i<m_polygons_count; ++i)
        m_polygons_holder->get_polygon(i)->update_points_with_single_matrix(_matrix);
    M_update_rectangular_border();
}

void Physical_Model_2D_Imprint::update_to_current_model_state()
{
    L_ASSERT(m_polygons_holder);

    for(unsigned int i=0; i<m_polygons_count; ++i)
    {
        Polygon& polygon = *m_polygons_holder->get_polygon(i);
        const Polygon& parent_polygon = *m_parent->get_polygon(i);

        for(unsigned int vert=0; vert<3; ++vert)
            polygon[vert] = parent_polygon[vert];

        m_polygons_holder->get_polygon(i)->calculate_center();
    }
    m_rect_border = m_parent->curr_rect_border();
}


const Physical_Model_2D* Physical_Model_2D_Imprint::get_parent() const
{
    return m_parent;
}

const Polygon* Physical_Model_2D_Imprint::get_polygon(unsigned int _index) const
{
    L_ASSERT(m_polygons_holder && _index < m_polygons_count);

    return m_polygons_holder->get_polygon(_index);
}

const Polygon_Holder_Base* Physical_Model_2D_Imprint::get_polygons() const
{
    return m_polygons_holder;
}

unsigned int Physical_Model_2D_Imprint::get_polygons_count() const
{
    return m_parent->get_polygons_count();
}

const Geometry_2D::Rectangular_Border& Physical_Model_2D_Imprint::curr_rect_border() const
{
    return m_rect_border;
}
