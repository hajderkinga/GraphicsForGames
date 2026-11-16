#include <iostream>
#include <cstdlib>
#include <random>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/epsilon.hpp>

const float THRESHOLD = (float)1.0e-5;

static glm::vec3 randomVec() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dist(1.0f, 5.0f);
	return glm::vec3(dist(gen), dist(gen), dist(gen));
}

static glm::mat3 randomMat3() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dist(-5.0f, 5.0f);
	glm::mat3 randomMat;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			randomMat[i][j] = dist(gen);
		}
	}
	return randomMat;
}

void ex1()
{
	glm::vec3 i, j, k;
	i = randomVec();
	j = randomVec();
	k = randomVec();

	glm::vec3 jxk = glm::cross(j, k);
	glm::vec3 left = glm::cross(i, jxk);
	std::cout << "i x (j x k) = (" << left.x << ", " << left.y << ", " << left.z << ")" << std::endl;

	float idotk = glm::dot(i, k);
	float idotj = glm::dot(i, j);

	glm::vec3 right = j * idotk - k * idotj;
	std::cout << "j * (i . k) - k * (i . j) = (" << right.x << ", " << right.y << ", " << right.z << ")" << std::endl;

	if (glm::all(glm::epsilonEqual(left, right, THRESHOLD)))
	{
		std::cout << "i × (j × k) = j * (i . k) - k * (i . j)" << std::endl;
	}
	else
	{
		std::cout << "i × (j × k) != j * (i . k) - k * (i . j)" << std::endl;
	}
}

bool createCameraFrame(const glm::vec3& view, const glm::vec3& up,
	glm::vec3& u, glm::vec3& v, glm::vec3& w)
{
	if (glm::length(view) < THRESHOLD) {
		return false;
	}

	v = glm::normalize(view);
	glm::vec3 upCrossView = glm::cross(up, v);
	if (glm::length(upCrossView) < THRESHOLD) {
		return false;
	}

	w = glm::normalize(upCrossView);
	u = glm::cross(v, w);
	return true;
}

void printCameraFrame(const glm::vec3& view, const glm::vec3& up)
{
	glm::vec3 u, v, w;
	std::cout << "view = (" << view.x << ", " << view.y << ", " << view.z << ")" << std::endl;
	std::cout << "up = (" << up.x << ", " << up.y << ", " << up.z << ")" << std::endl;

	if (!createCameraFrame(view, up, u, v, w)) {
		std::cout << "Cannot create a coordinate frame: view and up are parallel." << std::endl;
		return;
	}

	std::cout << "u = (" << u.x << ", " << u.y << ", " << u.z << ")" << std::endl;
	std::cout << "v = (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
	std::cout << "w = (" << w.x << ", " << w.y << ", " << w.z << ")" << std::endl;
}

void ex2()
{
	printCameraFrame(glm::vec3(2.0f, 3.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	printCameraFrame(glm::vec3(3.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	printCameraFrame(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 calculateRodriguesRotationFormula(const glm::vec3& v, const glm::vec3& a, float theta)
{
	glm::vec3 axis = glm::normalize(a);
	float thetaRadians = glm::radians(theta);

	return v * std::cos(thetaRadians)
		+ glm::cross(axis, v) * std::sin(thetaRadians)
		+ axis * glm::dot(axis, v) * (1.0f - std::cos(thetaRadians));
}

void ex3()
{
	const glm::vec3 axes[] = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 2.0f, 3.0f),
		glm::vec3(2.0f, 1.0f, 3.0f)
	};
	const glm::vec3 inputs[] = {
		glm::vec3(1.0f, 2.0f, 3.0f),
		glm::vec3(1.0f, 2.0f, 3.0f),
		glm::vec3(5.0f, 4.0f, 2.0f)
	};
	const float angles[] = { 90.0f, 45.0f, 30.0f };
	const glm::vec3 expected[] = {
		glm::vec3(2.5773503f, 0.8452995f, 2.5773503f),
		glm::vec3(1.0f, 2.0f, 3.0f),
		glm::vec3(3.3766054f, 5.1254307f, 2.7071195f)
	};

	for (int i = 0; i < 3; ++i) {
		glm::vec3 result = calculateRodriguesRotationFormula(inputs[i], axes[i], angles[i]);
		std::cout << "input: v = (" << inputs[i].x << ", " << inputs[i].y << ", " << inputs[i].z
			<< "), a = (" << axes[i].x << ", " << axes[i].y << ", " << axes[i].z
			<< "), theta = " << angles[i] << " degrees" << std::endl;
		std::cout << "expected: (" << expected[i].x << ", " << expected[i].y << ", " << expected[i].z << ")" << std::endl;
		std::cout << "result:   (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
	}
}

static bool printRotationTest(const glm::vec3& v, const glm::vec3& a, float theta,
	const glm::vec3& expected, const glm::vec3& result)
{
	std::cout << "input: v = (" << v.x << ", " << v.y << ", " << v.z
		<< "), a = (" << a.x << ", " << a.y << ", " << a.z
		<< "), theta = " << theta << " degrees" << std::endl;
	std::cout << "expected: (" << expected.x << ", " << expected.y << ", " << expected.z << ")" << std::endl;
	std::cout << "result:   (" << result.x << ", " << result.y << ", " << result.z << ")" << std::endl;
	const bool passed = glm::all(glm::epsilonEqual(result, expected, THRESHOLD));
	std::cout << (passed ? "PASS" : "FAIL") << std::endl;
	return passed;
}

glm::vec3 calculateRodriguesRotationMatrix(const glm::vec3& v, const glm::vec3& a, float theta)
{
	const glm::vec3 axis = glm::normalize(a);
	const float thetaRadians = glm::radians(theta);
	// GLM takes the columns of A, where A * v equals axis cross v.
	const glm::mat3 A(
		0.0f, axis.z, -axis.y,
		-axis.z, 0.0f, axis.x,
		axis.y, -axis.x, 0.0f
	);
	const glm::mat3 R = glm::mat3(1.0f) + std::sin(thetaRadians) * A
		+ (1.0f - std::cos(thetaRadians)) * (A * A);
	return R * v;
}

bool ex4()
{
	const glm::vec3 axes[] = { { 1, 0, 0 },{ 0, 1, 0 },{ 0, 0, 1 } };
	const float theta = 90.0f;
	// The first index selects the input vector and the second the rotation axis.
	const glm::vec3 expected[3][3] = {
		{ { 1, 0, 0 },{ 0, 0, -1 },{ 0, 1, 0 } },
		{ { 0, 0, 1 },{ 0, 1, 0 },{ -1, 0, 0 } },
		{ { 0, -1, 0 },{ 1, 0, 0 },{ 0, 0, 1 } }
	};

	bool passed = true;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			const glm::vec3 result = calculateRodriguesRotationMatrix(axes[i], axes[j], theta);
			passed = printRotationTest(axes[i], axes[j], theta, expected[i][j], result) && passed;
		}
	}
	return passed;
}

static bool matricesEqual(const glm::dmat3& left, const glm::dmat3& right)
{
	for (int column = 0; column < 3; ++column) {
		if (!glm::all(glm::epsilonEqual(left[column], right[column], static_cast<double>(THRESHOLD)))) {
			return false;
		}
	}
	return true;
}

bool validateTransposeProduct(const glm::dmat3& A, const glm::dmat3& B)
{
	return matricesEqual(glm::transpose(A * B), glm::transpose(B) * glm::transpose(A));
}

bool ex5()
{
	const glm::dmat3 A(randomMat3()), B(randomMat3());
	const bool passed = validateTransposeProduct(A, B);
	std::cout << "(AB)^T = B^T A^T: " << (passed ? "PASS" : "FAIL") << std::endl;
	return passed;
}

static bool isInvertible(const glm::dmat3& matrix)
{
	return std::abs(glm::determinant(matrix)) > THRESHOLD;
}

bool validateInverseProduct(const glm::dmat3& A, const glm::dmat3& B)
{
	if (!isInvertible(A) || !isInvertible(B)) {
		return false;
	}
	return matricesEqual(glm::inverse(A * B), glm::inverse(B) * glm::inverse(A));
}

bool ex6()
{
	glm::dmat3 A, B;
	do {
		A = glm::dmat3(randomMat3());
	} while (!isInvertible(A));
	do {
		B = glm::dmat3(randomMat3());
	} while (!isInvertible(B));

	const bool passed = validateInverseProduct(A, B);
	std::cout << "(AB)^-1 = B^-1 A^-1: " << (passed ? "PASS" : "FAIL") << std::endl;
	return passed;
}

int main()
{
	std::cout << "Exercise 1:" << std::endl;
	ex1();
	std::cout << std::endl;

	std::cout << "Exercise 2:" << std::endl;
	ex2();
	std::cout << std::endl;

	std::cout << "Exercise 3:" << std::endl;
	ex3();
	std::cout << std::endl;

	std::cout << "Exercise 4:" << std::endl;
	ex4();
	std::cout << std::endl;

	std::cout << "Exercise 5:" << std::endl;
	ex5();
	std::cout << std::endl;

	std::cout << "Exercise 6:" << std::endl;
	ex6();
	std::cout << std::endl;

	return 0;
}
