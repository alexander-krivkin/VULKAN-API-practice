#include "Geometry.h"


namespace ak::geometry
{
	uint32_t Primitive::getVerticesCount() const
	{
		return UINT32(vertices_.size());
	}

	std::vector<glm::vec3> Primitive::getVertices() const
	{
		return vertices_;
	}

	std::vector<Vertex3D> Primitive::getVertices(TopologyTypeFlag topologyType) const
	{
		std::vector<Vertex3D> ret{};

		for (auto& vertex : vertices_)
		{
			ret.push_back({ vertex, colors_[UINT32(topologyType)] });
		}

		return ret;
	}

	glm::vec4 Primitive::getColor(TopologyTypeFlag topologyType) const
	{
		return colors_[UINT32(topologyType)];
	}

	void Primitive::setColor(TopologyTypeFlag topologyType, glm::vec4 color)
	{
		colors_[UINT32(topologyType)] = color;
	}


	Circle2D::Circle2D(glm::vec3 center, float radius,
		const CoordinateSystem<glm::vec3>& coordinateSystem, uint32_t numFragments,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		center_ = FRACTION_3(center);
		radius_ = radius;
		coordinateSystem_ = coordinateSystem;
		numFragments_ = numFragments;
		colors_ = colors;

		glm::mat4 mattrix{};
		glm::vec3 pureCenter{ center };
		glm::vec4 pureRadius{ radius * coordinateSystem_.getNormalX(), 1.0f };
		glm::vec3 pureNormalZ{ coordinateSystem_.getNormalZ() };
		glm::vec3 tempVertex{};
		float deltaAngle = 360.0f / numFragments;

		for (uint32_t idx{}; idx < numFragments; idx++)
		{
			mattrix = glm::translate(glm::mat4(1.0f), pureCenter);
			mattrix = glm::rotate(mattrix, glm::radians(deltaAngle * idx), pureNormalZ);
			tempVertex = mattrix * pureRadius;
			vertices_.push_back(tempVertex);
		}
	}

	Circle2D::Circle2D(FractionVec3 center, Fraction radius,
		const CoordinateSystem<FractionVec3>& coordinateSystem, uint32_t numFragments,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		center_ = center;
		radius_ = radius;
		//coordinateSystem_ = coordinateSystem;			//////////// TODO Fraction
		numFragments_ = numFragments;
		colors_ = colors;

		glm::mat4 mattrix{};
		glm::vec3 pureCenter{ GLM_VEC_3(center) };
		glm::vec4 pureRadius{ GLM_VEC(radius) * coordinateSystem_.getNormalX(), 1.0f };
		glm::vec3 pureNormalZ{ coordinateSystem_.getNormalZ() };
		glm::vec3 tempVertex{};
		float deltaAngle = 360.0f / numFragments;

		for (uint32_t idx{}; idx < numFragments; idx++)
		{
			mattrix = glm::translate(glm::mat4(1.0f), pureCenter);
			mattrix = glm::rotate(mattrix, glm::radians(deltaAngle * idx), pureNormalZ);
			tempVertex = mattrix * pureRadius;
			vertices_.push_back(tempVertex);
		}
	}

	Circle2D::Circle2D(const Circle2D& rhs)
	{
		center_ = rhs.center_;
		radius_ = rhs.radius_;
		coordinateSystem_ = rhs.coordinateSystem_;
		numFragments_ = rhs.numFragments_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;
	}

	Circle2D& Circle2D::operator=(const Circle2D& rhs)
	{
		if (this == &rhs) { return *this; }

		center_ = rhs.center_;
		radius_ = rhs.radius_;
		coordinateSystem_ = rhs.coordinateSystem_;
		numFragments_ = rhs.numFragments_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;

		return *this;
	}

	std::vector<uint32_t> Circle2D::getVertexIndices(TopologyTypeFlag topologyType) const
	{
		std::vector<uint32_t> ret{};
		if (!vertices_.size()) { return ret; }

		switch (topologyType)
		{
		case TopologyTypeFlag::ePoints:
			for (uint32_t idx{}; idx < UINT32(vertices_.size()); idx++)
			{
				ret.push_back(idx);
			}
			break;

		case TopologyTypeFlag::eLines:
			for (uint32_t idx{}; idx < UINT32(vertices_.size() - 1); idx++)
			{
				ret.push_back(idx);
				ret.push_back(idx + 1);
			}
			ret.push_back(UINT32(vertices_.size() - 1));
			ret.push_back(0);
			break;
		}

		return ret;
	}


	Point::Point(glm::vec3 vertex,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		pureVertex_ = FRACTION_3(vertex);
		center_ = pureVertex_;
		colors_ = colors;
		vertices_.push_back(vertex);
	}

	Point::Point(FractionVec3 vertex,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		pureVertex_ = vertex;
		center_ = pureVertex_;
		colors_ = colors;
		vertices_.push_back(GLM_VEC_3(vertex));
	}

	Point::Point(const Point& rhs)
	{
		pureVertex_ = rhs.pureVertex_;
		center_ = rhs.center_;
		coordinateSystem_ = rhs.coordinateSystem_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;
	}

	Point& Point::operator=(const Point& rhs)
	{
		if (this == &rhs) {	return *this; }

		pureVertex_ = rhs.pureVertex_;
		center_ = rhs.center_;
		coordinateSystem_ = rhs.coordinateSystem_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;

		return *this;
	}

	std::vector<uint32_t> Point::getVertexIndices(TopologyTypeFlag topologyType) const
	{
		std::vector<uint32_t> ret{};
		if (!vertices_.size()) { return ret; }

		switch (topologyType)
		{
		case TopologyTypeFlag::ePoints:
			ret = { 0 };
			break;
		}

		return ret;
	}


	Segment::Segment(std::array<glm::vec3, 2> vertices,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		uint32_t idx{};
		for (auto& pureVertex : pureVertices_)
		{
			pureVertex = FRACTION_3(vertices[idx]);
			idx++;
		}

		center_ = pureVertices_[0];
		colors_ = colors;

		vertices_.resize(vertices.size());
		idx = 0;
		for (auto& vertex : vertices_)
		{
			vertex = vertices[idx];
			idx++;
		}
	}

	Segment::Segment(std::array<FractionVec3, 2> vertices,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		pureVertices_ = vertices;
		center_ = pureVertices_[0];
		colors_ = colors;

		vertices_.resize(vertices.size());
		uint32_t idx{};
		for (auto& vertex : vertices_)
		{
			vertex = GLM_VEC_3(vertices[idx]);
			idx++;
		}
	}

	Segment::Segment(const Segment& rhs)
	{
		pureVertices_ = rhs.pureVertices_;
		center_ = rhs.center_;
		coordinateSystem_ = rhs.coordinateSystem_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;
	}

	Segment& Segment::operator=(const Segment& rhs)
	{
		if (this == &rhs) { return *this; }

		pureVertices_ = rhs.pureVertices_;
		center_ = rhs.center_;
		coordinateSystem_ = rhs.coordinateSystem_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;

		return *this;
	}

	std::vector<uint32_t> Segment::getVertexIndices(TopologyTypeFlag topologyType) const
	{
		std::vector<uint32_t> ret{};
		if (!vertices_.size()) { return ret; }

		switch (topologyType)
		{
		case TopologyTypeFlag::ePoints:
		case TopologyTypeFlag::eLines:
			ret = { 0, 1 };
			break;
		}

		return ret;
	}


	Tetrahedron::Tetrahedron(glm::vec3 center, std::array<glm::vec3, 4> vertices,
		const CoordinateSystem<glm::vec3>& coordinateSystem,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		center_ = FRACTION_3(center);
		coordinateSystem_ = coordinateSystem;
		colors_ = colors;

		glm::mat4 mattrix{};
		glm::vec3 pureCenter{ center };
		glm::vec3 pureNormalZ{ coordinateSystem_.getNormalZ() };

		uint32_t idx{};
		for (auto& pureVertex : pureVertices_)
		{
			pureVertex = FRACTION_3(vertices[idx]);
			idx++;
		}

		vertices_.resize(vertices.size());
		idx = 0;
		for (auto& vertex : vertices_)
		{
			mattrix = glm::translate(glm::mat4(1.0f), pureCenter);
			vertex = coordinateSystem_.getMattrix() * mattrix * glm::vec4{ vertices[idx], 1.0f };
			idx++;
		}
	}

	//Tetrahedron::Tetrahedron(FractionVec3 center,std::array<FractionVec3, 4> vertices,
	//	const CoordinateSystem<FractionVec3>& coordinateSystem,
	//	std::array<glm::vec4, numTopologyTypes> colors)
	//{
	//	center_ = center;
	//	//coordinateSystem_ = coordinateSystem;			//////////// TODO Fraction
	//	colors_ = colors;

	//	glm::mat4 mattrix{};
	//	glm::vec3 pureCenter{ GLM_VEC_3(center) };
	//	glm::vec3 pureNormalZ{ coordinateSystem_.getNormalZ() };

	//	uint32_t idx{};
	//	for (auto& pureVertex : pureVertices_)
	//	{
	//		pureVertex = vertices[idx++];
	//	}

	//	vertices_.resize(vertices.size());
	//	idx = 0;
	//}

	Tetrahedron::Tetrahedron(const Tetrahedron& rhs)
	{
		pureVertices_ = rhs.pureVertices_;
		center_ = rhs.center_;
		coordinateSystem_ = rhs.coordinateSystem_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;
	}

	Tetrahedron& Tetrahedron::operator=(const Tetrahedron& rhs)
	{
		if (this == &rhs) { return *this; }

		pureVertices_ = rhs.pureVertices_;
		center_ = rhs.center_;
		coordinateSystem_ = rhs.coordinateSystem_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;

		return *this;
	}

	std::vector<uint32_t> Tetrahedron::getVertexIndices(TopologyTypeFlag topologyType) const
	{
		std::vector<uint32_t> ret{};
		if (!vertices_.size()) { return ret; }

		switch (topologyType)
		{
		case TopologyTypeFlag::ePoints:
			ret = { 0, 1, 2, 3 };
			break;

		case TopologyTypeFlag::eLines:
			ret = { 0, 1, 1, 2, 2, 0, 0, 3, 1, 3, 2, 3 };
			break;

		case TopologyTypeFlag::eTriangles:
			ret = { 0, 1, 2, 0, 1, 3, 1, 2, 3, 2, 0, 3 };
			break;
		}

		return ret;
	}


	Sphere::Sphere(glm::vec3 center, float radius,
		const CoordinateSystem<glm::vec3>& coordinateSystem, uint32_t numFragments,
		std::array<glm::vec4, numTopologyTypes> colors)
	{
		center_ = FRACTION_3(center);
		radius_ = radius;
		coordinateSystem_ = coordinateSystem;
		numFragments_ = numFragments;
		colors_ = colors;

		auto circlesCoordinateSystem = coordinateSystem_.getRotatedY(glm::radians(90.0f));
		float deltaAngle = 360.0f / numFragments;

		for (uint32_t idx{}; idx < (numFragments / 2); idx++)
		{
			circles_.push_back(Circle2D{ center, radius,
				circlesCoordinateSystem.getRotatedX(glm::radians(deltaAngle * idx)), numFragments, colors });
		}

		for (auto& circle : circles_)
		{
			auto addingVertices = circle.getVertices();
			vertices_.insert(vertices_.end(), addingVertices.begin(), addingVertices.end());
		}
	}

	//Sphere::Sphere(FractionVec3 center, Fraction radius, FractionVec3 normal, uint32_t numFragments,
	//	const std::array<glm::vec4, numTopologyTypes> colors)
	//{
	//}

	Sphere::Sphere(const Sphere& rhs)
	{
		center_ = rhs.center_;
		radius_ = rhs.radius_;
		coordinateSystem_ = rhs.coordinateSystem_;
		numFragments_ = rhs.numFragments_;
		circles_ = rhs.circles_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;
	}

	Sphere& Sphere::operator=(const Sphere& rhs)
	{
		if (this == &rhs) { return *this; }

		center_ = rhs.center_;
		radius_ = rhs.radius_;
		coordinateSystem_ = rhs.coordinateSystem_;
		numFragments_ = rhs.numFragments_;
		circles_ = rhs.circles_;
		colors_ = rhs.colors_;
		vertices_ = rhs.vertices_;

		return *this;
	}

	std::vector<uint32_t> Sphere::getVertexIndices(TopologyTypeFlag topologyType) const
	{
		std::vector<uint32_t> ret{};
		if (!vertices_.size()) { return ret; }

		switch (topologyType)
		{
		case TopologyTypeFlag::ePoints:
			for (uint32_t idx{}; idx < UINT32(vertices_.size()); idx++)
			{
				ret.push_back(idx);
			}
			break;

		case TopologyTypeFlag::eLines:
			for (uint32_t idx{}; idx < UINT32(vertices_.size() - 1); idx++)
			{
				ret.push_back(idx);
				ret.push_back(idx + 1);
			}
			ret.push_back(UINT32(vertices_.size() - 1));
			ret.push_back(0);

			for (uint32_t parallelIdx{ 1 }; parallelIdx < numFragments_; parallelIdx++)
			{
				if (parallelIdx == (numFragments_ / 2))	{ continue; }

				for (uint32_t meridianIdx{}; meridianIdx < (numFragments_ / 2) - 1; meridianIdx++)
				{
					ret.push_back(meridianIdx * numFragments_ + parallelIdx);
					ret.push_back((meridianIdx + 1) * numFragments_ + parallelIdx);
				}
				ret.push_back(((numFragments_ / 2) - 1) * numFragments_ + parallelIdx);
				ret.push_back(numFragments_ - parallelIdx);
			}
			break;

		case TopologyTypeFlag::eTriangles:
			for (uint32_t meridianIdx{ 0 }; meridianIdx < (numFragments_ / 2) - 1; meridianIdx++)
			{
				ret.push_back(0);
				ret.push_back((meridianIdx * numFragments_) + 1);
				ret.push_back(((meridianIdx + 1) * numFragments_) + 1);

				ret.push_back(0);
				ret.push_back((meridianIdx * numFragments_) + (numFragments_ - 1));
				ret.push_back(((meridianIdx + 1) * numFragments_) + (numFragments_ - 1));

				ret.push_back(numFragments_ / 2);
				ret.push_back((meridianIdx * numFragments_) + ((numFragments_ / 2) - 1));
				ret.push_back(((meridianIdx + 1) * numFragments_) + ((numFragments_ / 2) - 1));

				ret.push_back(numFragments_ / 2);
				ret.push_back((meridianIdx * numFragments_) + ((numFragments_ / 2) + 1));
				ret.push_back(((meridianIdx + 1) * numFragments_) + ((numFragments_ / 2) + 1));
			}

			ret.push_back(0);
			ret.push_back((numFragments_ / 2) * numFragments_ - 1);
			ret.push_back(1);

			ret.push_back(0);
			ret.push_back(((numFragments_ / 2) - 1) * numFragments_ + 1);
			ret.push_back(numFragments_ - 1);

			ret.push_back(numFragments_ / 2);
			ret.push_back(((numFragments_ / 2) - 1) * numFragments_ + (numFragments_ / 2) + 1);
			ret.push_back((numFragments_ / 2) - 1);

			ret.push_back(numFragments_ / 2);
			ret.push_back(((numFragments_ / 2) - 1) * numFragments_ + (numFragments_ / 2) - 1);
			ret.push_back((numFragments_ / 2) + 1);

			for (uint32_t parallelIdx{ 1 }; parallelIdx < (numFragments_ - 1); parallelIdx++)
			{
				if ((parallelIdx > ((numFragments_ / 2) - 2)) && (parallelIdx < ((numFragments_ / 2) + 1)))
				{ continue; }

				for (uint32_t meridianIdx{ 0 }; meridianIdx < (numFragments_ / 2) - 1; meridianIdx++)
				{
					ret.push_back((meridianIdx * numFragments_) + parallelIdx);
					ret.push_back((meridianIdx * numFragments_) + parallelIdx + 1);
					ret.push_back(((meridianIdx + 1) * numFragments_) + parallelIdx + 1);

					ret.push_back(((meridianIdx + 1) * numFragments_) + parallelIdx + 1);
					ret.push_back(((meridianIdx + 1) * numFragments_) + parallelIdx);
					ret.push_back((meridianIdx * numFragments_) + parallelIdx);
				}

				ret.push_back((numFragments_ / 2) * numFragments_ - parallelIdx);
				ret.push_back((numFragments_ / 2) * numFragments_ - parallelIdx - 1);
				ret.push_back(parallelIdx + 1);

				ret.push_back(parallelIdx);
				ret.push_back((numFragments_ / 2) * numFragments_ - parallelIdx);
				ret.push_back(parallelIdx + 1);

			}
			break;
		}

		return ret;
	}
}