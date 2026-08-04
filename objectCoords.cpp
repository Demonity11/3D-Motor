#include "objectCoords.h"
#include "draw_utils.h"
#include "Window.h"
#include "Object.h"
#include "utilities.h"
#include "Context.h"

void getNewCoordSystem(glm::vec3& direction, glm::vec3& right, glm::vec3& up)
{
	direction = glm::normalize(direction);

	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };

	// changes worldUp accordingly with the direction vector, because the cross product between vectors with the same direction is a null vector 
	float cosTheta = glm::dot(direction, worldUp);
	if (glm::abs(cosTheta) > 0.999f)
	{
		if (cosTheta > 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

		if (cosTheta < 0.0f)
			worldUp = glm::vec3(0.0f, 0.0f, -1.0f);
	}

	right = glm::normalize(glm::cross(direction, worldUp));
	up = glm::cross(right, direction);
}

int getCilinderVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, float radius, std::vector<float>& vertexData)
{
	size_t verticesBefore{ vertexData.size() / 7 };

	glm::vec3 direction{ p - p0 };

	float length{ glm::length(direction) };

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	constexpr int maxLines{ 360 };
	constexpr float linesDensity{ 360.0f / static_cast<float>(maxLines) };

	vertexData.reserve(vertexData.size() + static_cast<size_t>(maxLines * 14));

	// this loop does maxLines * 14
	// for maxLines = 72, 72 * 14 = 1008 pushbacks 
	// vertexCount = 1008 / 7 = 144
	for (int line{ 0 }; line < maxLines; ++line)
	{
		float rad{ glm::radians(static_cast<float>(line) * linesDensity) };

		glm::vec3 a{ p0 + radius * cos(rad) * right + radius * sin(rad) * up };
		glm::vec3 b{ a + length * direction };

		vertexData.push_back(a.x);
		vertexData.push_back(a.y);
		vertexData.push_back(a.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(b.x);
		vertexData.push_back(b.y);
		vertexData.push_back(b.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);
	}

	size_t verticesAfter{ vertexData.size() / 7 };

	return static_cast<int>(verticesAfter - verticesBefore);
}

int getConeVertices(glm::vec3 direction, glm::vec3 apex, glm::vec4 color, float radius, float height, std::vector<float>& vertexData)
{
	size_t verticesBefore{ vertexData.size() / 7 };

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	glm::vec3 baseCenter{ apex - direction * height };

	auto transformToWorld = [&](const glm::vec3 localP) -> glm::vec3
		{
			return baseCenter + (localP.x * right) + (localP.y * direction) + (localP.z * up);
		};

	constexpr int N{ 360 };

	glm::vec3 apexLocal{ 0.0f, height, 0.0f };

	glm::vec3 apexWorld{ transformToWorld(apexLocal) };

	vertexData.reserve(vertexData.size() + static_cast<size_t>(N * 28));

	// this loop does 28 * N pushbacks
	// for N = 64, 1792 pushbacks
	// vertexCount = 1792 / 7 = 256
	for (int i{ 0 }; i < N; ++i)
	{
		float angle{ i * glm::radians(360.0f) / static_cast<float>(N) };
		glm::vec3 pBaseLocal{ radius * cos(angle), 0.0f, radius * sin(angle) };
		
		int iNext{ i + 1 };
		float angleNext{ iNext * glm::radians(360.0f) / static_cast<float>(N) };
		glm::vec3 pBaseNextLocal{ radius * cos(angleNext), 0.0f, radius * sin(angleNext) };

		glm::vec3 pBaseWorld{ transformToWorld(pBaseLocal) };
		glm::vec3 pBaseNextWorld{ transformToWorld(pBaseNextLocal) };

		// side lines
		vertexData.push_back(apexWorld.x);
		vertexData.push_back(apexWorld.y);
		vertexData.push_back(apexWorld.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(pBaseWorld.x);
		vertexData.push_back(pBaseWorld.y);
		vertexData.push_back(pBaseWorld.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		// base lines
		vertexData.push_back(pBaseWorld.x);
		vertexData.push_back(pBaseWorld.y);
		vertexData.push_back(pBaseWorld.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(pBaseNextWorld.x);
		vertexData.push_back(pBaseNextWorld.y);
		vertexData.push_back(pBaseNextWorld.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);
	}

	size_t verticesAfter{ vertexData.size() / 7 };

	return static_cast<int>(verticesAfter - verticesBefore);
}

int getLineVertices(glm::vec3 point, glm::vec3 dVecP0, glm::vec3 dVecP, glm::vec4 color, float radius, std::vector<float>& vertexData)
{
	size_t verticesBefore{ vertexData.size() / 7 };

	glm::vec3 direction{ dVecP - dVecP0 };

	float length{ glm::length(direction) };

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	constexpr int maxLines{ 360 };
	constexpr float linesDensity{ 360.0f / static_cast<float>(maxLines) };

	vertexData.reserve(vertexData.size() + static_cast<size_t>(maxLines * 14));

	// this loop does maxLines * 14
	// for maxLines = 72, 72 * 14 = 1008 pushbacks 
	// vertexCount = 1008 / 7 = 144
	for (int line{ 0 }; line < maxLines; ++line)
	{
		float rad{ glm::radians(static_cast<float>(line) * linesDensity) };

		glm::vec3 a{ point + radius * cos(rad) * right + radius * sin(rad) * up }; a -= direction * length * 10.0f;
		glm::vec3 b{ a + length * direction * 20.0f};

		vertexData.push_back(a.x);
		vertexData.push_back(a.y);
		vertexData.push_back(a.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(b.x);
		vertexData.push_back(b.y);
		vertexData.push_back(b.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);
	}

	size_t verticesAfter{ vertexData.size() / 7 };

	return static_cast<int>(verticesAfter - verticesBefore);
}

int getRingsVertices(glm::vec3 p0, glm::vec3 p, glm::vec4 color, std::vector<float>& vertexData)
{
	size_t verticesBefore{ vertexData.size() / 7 };

	glm::vec3 direction{ p - p0 };

	float ringWidth{ glm::length(direction) };

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	constexpr int maxLines{ 360 };
	constexpr float linesDensity{ 360.0f / static_cast<float>(maxLines) };
	constexpr float ringRadius{ 0.0011f };
	constexpr float stride{ 0.1f };
	constexpr int ringCount{ 19 };

	vertexData.reserve(vertexData.size() + static_cast<size_t>(ringCount * maxLines * 14));

	// this loop does ringCount * (360 / linesDensity) * 14
	// for ringCount = 20, linesDensity = 5
	// this loop does 20 * 72 * 14 = 20160 pushbacks
	// vertexCount = 20160 / 7 = 2880
	for (int i{ 0 }; i < ringCount; ++i)
	{
		p0 += direction * stride;
		
		if (i + 1 == (ringCount + 1) / 2)
		{
			continue;
		}

		for (int line{ 0 }; line < maxLines; ++line)
		{
			float rad{ glm::radians(static_cast<int>(line) * linesDensity) };

			glm::vec3 a{ p0 + ringRadius * cos(rad) * right + ringRadius * sin(rad) * up };
			glm::vec3 b{ a + ringWidth * direction };

			vertexData.push_back(a.x);
			vertexData.push_back(a.y);
			vertexData.push_back(a.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);

			vertexData.push_back(b.x);
			vertexData.push_back(b.y);
			vertexData.push_back(b.z);
			vertexData.push_back(color.x);
			vertexData.push_back(color.y);
			vertexData.push_back(color.z);
			vertexData.push_back(color.w);
		}
	}

	size_t verticesAfter{ vertexData.size() / 7 };

	return static_cast<int>(verticesAfter - verticesBefore);
}

int getSphereVertices(glm::vec3 translation, glm::vec4 color, float radius, std::vector<float>& vertexData)
{
	size_t verticesBefore{ vertexData.size() / 7 };

	constexpr int maxStacks{ 120 };
	constexpr int maxSlices{ 144 };

	constexpr float deltaPhi{ glm::radians(180.f / static_cast<float>(maxStacks)) };

	constexpr float deltaTheta{ glm::radians(360.f / static_cast<float>(maxSlices)) };

	vertexData.reserve(vertexData.size() + static_cast<size_t>(maxStacks * maxSlices * 28));

	// this loop does 60 * 72 * 28 = 120960 pushbacks
	// vertexCount = 120960 / 7 = 17280
	for (int stack{ 0 }; stack < maxStacks; ++stack)
	{
		float phi{ static_cast<float>(stack) * deltaPhi};
		float phiNext{ static_cast<float>(stack + 1) * deltaPhi };

		for (int slice{ 0 }; slice < maxSlices; ++slice)
		{
			float theta{ static_cast<float>(slice) * deltaTheta };
			float thetaNext{ static_cast<float>(slice + 1) * deltaTheta };

			glm::vec3 p0
			{
				translation.x + radius * sin(phi) * cos(theta),
				translation.y + radius * cos(phi),
				translation.z + radius * sin(phi) * sin(theta)
			};

			glm::vec3 pRight
			{
				translation.x + radius * sin(phi) * cos(thetaNext),
				translation.y + radius * cos(phi),
				translation.z + radius * sin(phi) * sin(thetaNext)
			};

			glm::vec3 pBottom
			{
				translation.x + radius * sin(phiNext) * cos(theta),
				translation.y + radius * cos(phiNext),
				translation.z + radius * sin(phiNext) * sin(theta)
			};

			if (stack > 0)
			{
				vertexData.push_back(p0.x); 
				vertexData.push_back(p0.y); 
				vertexData.push_back(p0.z);
				vertexData.push_back(color.x); 
				vertexData.push_back(color.y); 
				vertexData.push_back(color.z); 
				vertexData.push_back(color.w);

				vertexData.push_back(pRight.x); 
				vertexData.push_back(pRight.y); 
				vertexData.push_back(pRight.z);
				vertexData.push_back(color.x); 
				vertexData.push_back(color.y); 
				vertexData.push_back(color.z); 
				vertexData.push_back(color.w);
			}

			vertexData.push_back(p0.x); 
			vertexData.push_back(p0.y); 
			vertexData.push_back(p0.z);
			vertexData.push_back(color.x); 
			vertexData.push_back(color.y); 
			vertexData.push_back(color.z); 
			vertexData.push_back(color.w);

			vertexData.push_back(pBottom.x); 
			vertexData.push_back(pBottom.y);
			vertexData.push_back(pBottom.z);
			vertexData.push_back(color.x); 
			vertexData.push_back(color.y); 
			vertexData.push_back(color.z); 
			vertexData.push_back(color.w);
		}
	}

	size_t verticesAfter{ vertexData.size() / 7 };

	return static_cast<int>(verticesAfter - verticesBefore);
}

int getPlaneVertices(glm::vec3 normalP0, glm::vec3 normalP, glm::vec3 point, glm::vec4 color, std::vector<float>& vertexData)
{
	size_t verticesBefore{ vertexData.size() / 7 };

	glm::vec3 direction{ normalP - normalP0 };

	const float length{ glm::length(direction) };

	glm::vec3 right{};
	glm::vec3 up{};

	getNewCoordSystem(direction, right, up);

	//glm::mat3 R{ right, direction, up };

	std::vector planeVertices
	{
		 1.0f, 0.0f, -1.0f,
		 1.0f, 0.0f,  1.0f,
		-1.0f, 0.0f, -1.0f,

		 1.0f, 0.0f,  1.0f,
		-1.0f, 0.0f,  1.0f,
		-1.0f, 0.0f, -1.0f
	};

	vertexData.reserve(vertexData.size() + static_cast<size_t>(6 * 7));

	// this loop does 6 * 7 = 42 pushbacks
	// vertexCount = 6
	for (int i{ 0 }; i < planeVertices.size(); i += 3)
	{
		glm::vec3 p
		{
			planeVertices[i] * right.x + planeVertices[i + 2] * up.x + point.x,
			planeVertices[i] * right.y + planeVertices[i + 2] * up.y + point.y,
			planeVertices[i] * right.z + planeVertices[i + 2] * up.z + point.z
		};

		vertexData.push_back(p.x);
		vertexData.push_back(p.y);
		vertexData.push_back(p.z);

		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);
	}

	size_t verticesAfter{ vertexData.size() / 7 };

	return static_cast<int>(verticesAfter - verticesBefore);
}

int getGridVertices(std::vector<float>& vertexData)
{
	size_t verticesBefore{ vertexData.size() / 7 };

	glm::vec3 p0Horizontal{ -1.0f, 0.0f, -1.0f };
	glm::vec3 pHorizontal {  1.0f, 0.0f, -1.0f };

	glm::vec3 p0Vertical  { -1.0f, 0.0f, -1.0f };
	glm::vec3 pVertical   { -1.0f, 0.0f,  1.0f };

	glm::vec4 color{ 0.0f, 0.0f, 0.0f, 0.5f };

	constexpr float stride{ 0.1f };
	constexpr int lineCount{ 21 };
	
	vertexData.reserve(vertexData.size() + static_cast<size_t>(lineCount * 28));

	// this loop does 21 * 28 = 588 pushbacks
	// vertexCount = 84
	for (int i{ 0 }; i < lineCount; ++i)
	{
		vertexData.push_back(p0Horizontal.x);
		vertexData.push_back(p0Horizontal.y);
		vertexData.push_back(p0Horizontal.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(pHorizontal.x);
		vertexData.push_back(pHorizontal.y);
		vertexData.push_back(pHorizontal.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(p0Vertical.x);
		vertexData.push_back(p0Vertical.y);
		vertexData.push_back(p0Vertical.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		vertexData.push_back(pVertical.x);
		vertexData.push_back(pVertical.y);
		vertexData.push_back(pVertical.z);
		vertexData.push_back(color.x);
		vertexData.push_back(color.y);
		vertexData.push_back(color.z);
		vertexData.push_back(color.w);

		p0Horizontal.z += stride;
		pHorizontal.z += stride;

		p0Vertical.x += stride;
		pVertical.x += stride;
	}

	size_t verticesAfter{ vertexData.size() / 7 };

	return static_cast<int>(verticesAfter - verticesBefore);
}

void getEnvironmentVertices(std::vector<float>& vertexData, bool firstRun)
{
	std::vector planeVertices
	{
		 1.0f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
		 1.0f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
		-1.0f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.1f,

		 1.0f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
		-1.0f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f, 0.1f,
		-1.0f, 0.0f, -1.0f,  0.0f, 0.0f, 0.0f, 0.1f
	};

	if (firstRun)
	{
		Eval::Plane plane{ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };

		Object obj{ "GRID_PLANE", Object::Plane, GL_TRIANGLES, plane, glm::vec4{0.0f, 0.0f, 0.0f, 0.1f} };
		createObject(obj, static_cast<int>(planeVertices.size()) / 7);
	}
	
	vertexData = std::move(planeVertices);

	std::vector axisVertices
	{
		-1.0f, 0.0f, 0.0f,
		 1.0f, 0.0f, 0.0f,

		 0.0f, -1.0f, 0.0f,
		 0.0f,  1.0f, 0.0f,

		 0.0f, 0.0f, -1.0f,
		 0.0f, 0.0f,  1.0f
	};

	std::vector axisColors
	{
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f
	};

	char axisPrefix{ 'X' };

	glm::vec3 axisPos{ 0.0f, 0.5f, 0.0f };

	// this loop does 3 * 1008 = 3024 pushbacks
	for (int v{ 0 }, c{ 0 }; v < axisVertices.size(); v += 6, c += 4)
	{
		glm::vec3 a    { axisVertices[v], axisVertices[v + 1], axisVertices[v + 2] };
		glm::vec3 b    { axisVertices[v + 3], axisVertices[v + 4], axisVertices[v + 5] };
		glm::vec4 color{ axisColors[c], axisColors[c + 1], axisColors[c + 2], axisColors[c + 3] };

		const float cilinderLength{ glm::length(b - a) };

		glm::vec3 direction{ glm::normalize(b - a) };

		constexpr float radius{ 0.001f };
		constexpr float coneRadius{ radius * 4.0f };
		constexpr float coneHeight{ 0.025f };

		auto newB{ a + (cilinderLength - coneHeight) * direction };

		int vCountCilinder{ getCilinderVertices(a, newB, color, radius, vertexData) };
		int vCountCone{ getConeVertices(direction, b, color, coneRadius, coneHeight, vertexData) };

		if (firstRun)
		{
			Eval::Vector axisVec{ a, b };
			Object obj{ std::string(1, axisPrefix) + "_AXIS", Object::Vector, GL_LINES, axisVec, color };
			createObject(obj, vCountCilinder + vCountCone);

			++axisPrefix;
		}
	}

	constexpr float ringWidth{ 0.002f };
	std::vector ringVertices
	{
		-1.0f, 0.0f, 0.0f,
		-1.0f + ringWidth, 0.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f + ringWidth, 0.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f + ringWidth
	};

	glm::vec4 ringColor{ 0.0f, 0.0f, 0.0f, 1.0f };

	axisPrefix = 'X';

	// this loop does 3 * 20160 = 60480 pushbacks
	for (size_t v{ 0 }; v < axisVertices.size(); v += 6)
	{
		glm::vec3 a{ ringVertices[v], ringVertices[v + 1], ringVertices[v + 2] };
		glm::vec3 b(ringVertices[v + 3], ringVertices[v + 4], ringVertices[v + 5]);

		int vCountRing{ getRingsVertices(a, b, ringColor, vertexData) };

		if (firstRun)
		{
			Eval::Segment axisSeg{ a, b };
			Object obj{ std::string(1, axisPrefix) + "_AXIS_RINGS", Object::Segment, GL_LINES, axisSeg, ringColor };
			createObject(obj, vCountRing);

			++axisPrefix;
		}
	}

	// this execution does 21 * 28 = 588 pushbacks
	int vCountGrid{ getGridVertices(vertexData) };

	if (firstRun)
	{
		Eval::Segment seg{ glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f) };

		Object obj{ "GRID_LINES", Object::Segment, GL_LINES, seg, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f) };
		createObject(obj, vCountGrid);
	}
}