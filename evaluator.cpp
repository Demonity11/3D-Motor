#include "evaluator.h"
#include "Object.h"
#include "utilities.h"
#include "objectCoords.h"
#include "Random.h"

RuntimeValue evaluator(const std::vector<Node>& nodes, const std::vector<Object>& object, int nodeIdx)
{
    using Context::RuntimeError;
    using Context::ErrorSeverity;

    if (nodes.empty())
    {
        return RuntimeError
        { 
            "Semantics Warning at col 0: Nodes vector is empty.",
            ErrorSeverity::Warning,
            0,
            0
        };
    }

    const Node& node{ nodes[nodeIdx] };

    if (node.type == Node::Literal)
    {
        std::optional<float> literal{ convertSVToFloat(node.content) };

        if (!literal.has_value())
        {
            return RuntimeError
            { 
                "Semantics Error at col " + std::to_string(node.charPosition + 1) + ": Invalid float format.",
                ErrorSeverity::Error,
                node.charPosition,
                node.content.length()
            };
        }

        return *literal;
    }

    else if (node.type == Node::Function)
    {
        bool funcExist{ false };
        for (const auto& func : Context::funcOverloads)
        {
            if (node.content == func.name) 
            {
                funcExist = true;
                break;
            }
        }

        if (!funcExist)
        {
            return RuntimeError
            { 
                "Semantics Error at col " + std::to_string(node.charPosition + 1) + ": Function '" + std::string(node.content) + "' does not exist.",
                ErrorSeverity::Error,
                node.charPosition,
                node.content.length()
            };
        }

        std::vector<RuntimeValue> args{};
        for (int childIdx : node.children)
        {
            if (childIdx == -1) break;

            RuntimeValue childVal{ evaluator(nodes, object, childIdx) };

            if (std::holds_alternative<RuntimeError>(childVal))
            {
                return childVal;
            }

            args.push_back(childVal);
        }

        if (node.content == "Point")
        {
            return evaluatePointFunc(args, node, nodes);
        }
        else if (node.content == "Vector")
        {
            return evaluateVectorFunc(args, node, nodes);
        }
        else if (node.content == "Cross")
        {
            return evaluateCrossFunc(args, node, nodes);
        }
        else if (node.content == "Segment")
        {
            return evaluateSegmentFunc(args, node, nodes);
        }
        else if (node.content == "Line")
        {
            return evaluateLineFunc(args, node, nodes);
        }
        else if (node.content == "Plane")
        {
            return evaluatePlaneFunc(args, node, nodes);
        }
        else if (node.content == "Intersect")
        {
            return evaluateIntersectFunc(args, node, nodes);
        }

        return RuntimeError
        { 
            "Semantics Error at col " + std::to_string(node.charPosition + 1) + ": Function '" + std::string(node.content) + "' not found.",
            ErrorSeverity::Error,
            node.charPosition,
            node.content.length()
        };
    }

    else if (node.type == Node::Exp1 || node.type == Node::Exp2)
    {
        std::string_view op{ node.content };

        std::array<RuntimeValue, 2> operands{};
        for (size_t i{ 0 }; i < operands.size(); ++i)
        {
            int childIdx{ node.children[i] };
            if (childIdx == -1) break;

            RuntimeValue operand{ evaluator(nodes, object, childIdx) };

            if (std::holds_alternative<RuntimeError>(operand))
            {
                return operand;
            }

            operands[i] = operand;
        }

        if (op == "+")
        {
            return evaluateSumOperator(operands);
        }

        else if (op == "-")
        {
            return evaluateSubtractionOperator(operands);
        }

        else if (op == "*")
        {
            return evaluateMultiplicationOperator(operands);
        }

        else
        {
            return Context::RuntimeError{};
        }
    }

    else if (node.type == Node::Variable)
    {
        int objIdx{ searchObjectIndexByName(node.content, object) };
        if (objIdx == -1)
        {
            return RuntimeError
            { 
                "Semantics Error at col " + std::to_string(node.charPosition + 1) + ": Variable '" + std::string(node.content) + "' does not exist.",
                ErrorSeverity::Error,
                node.charPosition,
                node.content.length()
            };
        }

        const Object& obj{ object[objIdx] };

        return obj.getComponents();
    }

    return RuntimeError
    { 
        "Semantics Error at col " + std::to_string(node.charPosition + 1) + ": Unknown Token",
        ErrorSeverity::Error,
        node.charPosition,
        node.content.length()
    };
}

std::optional<float> convertSVToFloat(std::string_view sv)
{
    float value{};

    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

    if (ec == std::errc{})
    {
        return value;
    }

    return {};
}

void printRuntimeValue(const RuntimeValue& value)
{
    std::visit(overloaded
        {
        [](float f)
        {
            std::cout << "Literal:\t" << f << '\n';
        },
        [](const glm::vec3& point)
        {
            std::cout << "Point:\t(" << point << ")\n";
        },
        [](const Eval::Vector& vector)
        {
            std::cout << "Vector Origin:\t(" << vector.origin << ")\n";
            std::cout << "Vector Head:\t(" << vector.head << ")\n";
        },
        [](const Eval::Segment& segment)
        {
            std::cout << "Segment A:\t(" << segment.A << ")\n";
            std::cout << "Segment B:\t(" << segment.B << ")\n";
        },
        [](const Eval::Line& line)
        {
            std::cout << "Point:\t(" << line.point << ")\n";
            std::cout << "Direction Vector Origin:\t(" << line.dVecOrigin << ")\n";
            std::cout << "Direction Vector Head:\t(" << line.dVecHead << ")\n";
        },
        [](const Eval::Plane& plane)
        {
            std::cout << "Point:\t(" << plane.point << ")\n";
            std::cout << "Normal Vector Origin:\t(" << plane.normalOrigin << ")\n";
            std::cout << "Normal Vector Head:\t(" << plane.normalHead << ")\n";
        },
        [](const Eval::IPoint& iPoint)
        {
            std::cout << "Intersect:\t(" << iPoint.point << ")\n";
        },
        [](const Eval::ILine& iLine)
        {
            std::cout << "Intersect\n";
            std::cout << "Point:\t(" << iLine.line.point << ")\n";
            std::cout << "Direction Vector Origin:\t(" << iLine.line.dVecOrigin << ")\n";
            std::cout << "Direction Vector Head:\t(" << iLine.line.dVecHead << ")\n";
        },
        [](const Context::RuntimeError& error)
        {
            std::cerr << error.message << '\n';
        }

        }, value);
}

RuntimeValue evaluateSumOperator(const std::array<RuntimeValue, 2>& operands)
{
    if (auto* f0{ std::get_if<float>(&operands[0]) }, * f1{ std::get_if<float>(&operands[1]) }; f0 && f1)
    {
        return *f0 + *f1;
    }

    else if (auto* v0{ std::get_if<Eval::Vector>(&operands[0]) }, * v1{ std::get_if<Eval::Vector>(&operands[1]) }; v0 && v1)
    {
        return Eval::Vector{ v0->origin + v1->origin, v0->head + v1->head };
    }

    else if (std::holds_alternative<Eval::Vector>(operands[0]) &&
        (std::holds_alternative<glm::vec3>(operands[1]) || std::holds_alternative<Eval::IPoint>(operands[1])))
    {
        const Eval::Vector& v{ std::get<Eval::Vector>(operands[0]) };
        const std::optional<glm::vec3>& p{ extractPoint(operands[1]) };

        if (p)
        {
            return glm::vec3{ (v.head - v.origin) + (*p) };
        }
    }

    else if ((std::holds_alternative<glm::vec3>(operands[0]) || std::holds_alternative<Eval::IPoint>(operands[0])) &&
              std::holds_alternative<Eval::Vector>(operands[1])
        )
    {
        const std::optional<glm::vec3>& p{ extractPoint(operands[0]) };
        const Eval::Vector& v{ std::get<Eval::Vector>(operands[1]) };

        if (p)
        {
            return glm::vec3{ (v.head - v.origin) + (*p) };
        }
    }

    return Context::RuntimeError{};
}

RuntimeValue evaluateSubtractionOperator(const std::array<RuntimeValue, 2>& operands)
{
    if (auto* f0{ std::get_if<float>(&operands[0]) }, * f1{ std::get_if<float>(&operands[1]) }; f0 && f1)
    {
        return *f0 - *f1;
    }

    else if (auto* v0{ std::get_if<Eval::Vector>(&operands[0]) }, * v1{ std::get_if<Eval::Vector>(&operands[1]) }; v0 && v1)
    {
        return Eval::Vector{ v0->origin - v1->origin, v0->head - v1->head };
    }

    else if ((std::holds_alternative<glm::vec3>(operands[0]) || std::holds_alternative<Eval::IPoint>(operands[0])) &&
        std::holds_alternative<Eval::Vector>(operands[1])
        )
    {
        const std::optional<glm::vec3>& p{ extractPoint(operands[0]) };
        const Eval::Vector& v{ std::get<Eval::Vector>(operands[1]) };

        if (p)
        {
            return glm::vec3{ (*p) - (v.head - v.origin) };
        }
    }

    else if ((std::holds_alternative<glm::vec3>(operands[0]) || std::holds_alternative<Eval::IPoint>(operands[0])) &&
        (std::holds_alternative<glm::vec3>(operands[1]) || std::holds_alternative<Eval::IPoint>(operands[1]))
        )
    {
        const std::optional<glm::vec3>& p0{ extractPoint(operands[0]) };
        const std::optional<glm::vec3>& p1{ extractPoint(operands[1]) };

        if (p0 && p1)
        {
            return Eval::Vector{ *p1, *p0 };
        }
    }

    return Context::RuntimeError{};
}

RuntimeValue evaluateMultiplicationOperator(const std::array<RuntimeValue, 2>& operands)
{
    if (auto* f0{ std::get_if<float>(&operands[0]) }, * f1{ std::get_if<float>(&operands[1]) }; f0 && f1)
    {
        return (*f0) * (*f1);
    }

    else if (std::holds_alternative<float>(operands[0]) && std::holds_alternative<Eval::Vector>(operands[1]))
    {
        float f{ std::get<float>(operands[0]) };
        const Eval::Vector& v{ std::get<Eval::Vector>(operands[1]) };

        return Eval::Vector{ f * v.origin, f * v.head };
    }

    else if (std::holds_alternative<Eval::Vector>(operands[0]) && std::holds_alternative<float>(operands[1]))
    {
        const Eval::Vector& v{ std::get<Eval::Vector>(operands[0]) };
        float f{ std::get<float>(operands[1]) };

        return Eval::Vector{ f * v.origin, f * v.head };
    }

    else if (std::holds_alternative<float>(operands[0]) && 
        (std::holds_alternative<glm::vec3>(operands[1]) || std::holds_alternative<Eval::IPoint>(operands[1]))
        )
    {
        float f{ std::get<float>(operands[0]) };
        const std::optional<glm::vec3>& p{ extractPoint(operands[1]) };

        if (p)
        {
            return f * (*p);
        }
    }

    else if ((std::holds_alternative<glm::vec3>(operands[0]) || std::holds_alternative<Eval::IPoint>(operands[0])) &&
        std::holds_alternative<float>(operands[1])
        )
    {
        const std::optional<glm::vec3>& p{ extractPoint(operands[0]) };
        float f{ std::get<float>(operands[1]) };

        if (p)
        {
            return f * (*p);
        }
    }

    else if (auto* v0{ std::get_if<Eval::Vector>(&operands[0]) }, * v1{ std::get_if<Eval::Vector>(&operands[1]) }; v0 && v1)
    {
        return glm::dot(v0->head - v0->origin, v1->head - v1->origin);
    }

    return Context::RuntimeError{};
}

RuntimeValue evaluatePointFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)
{
    const std::array<int, 3>& cIdx{ node.children };

    if (args.size() == 3)
    {
        if (std::holds_alternative<float>(args[0]) &&
            std::holds_alternative<float>(args[1]) &&
            std::holds_alternative<float>(args[2]))
        {
            float x{ std::get<float>(args[0]) };
            float y{ std::get<float>(args[1]) };
            float z{ std::get<float>(args[2]) };

            glm::vec3 point{ x, y, z };

            return point;
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };
        const Object::Type t2{ deduceTypeByIdentifierName(nodes[cIdx[2]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Point accepts (Number, Number, Number) arguments, (" +
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ", " +
            getStringFunctionType(t2) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    return Context::RuntimeError
    {
        "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
        "Point only suports 3 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        node.charPosition + node.content.length(),
        1
    };
}

RuntimeValue evaluateVectorFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)
{
    const std::array<int, 3>& cIdx{ node.children };

    if (args.size() == 1)
    {
        auto p{ extractPoint(args[0]) };

        if (p)
        {
            Eval::Vector vector{ glm::vec3(0.0f), *p};

            vector.pTypes[0] = Object::Null;
            vector.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);

            return vector;
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Vector accepts (Point) argument, (" +
            getStringFunctionType(t0) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    else if (args.size() == 2)
    {
        auto p0{ extractPoint(args[0]) };
        auto p1{ extractPoint(args[1]) };

        if (p0 && p1)
        {
            Eval::Vector vector{ *p0,*p1 };

            vector.pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
            vector.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

            return vector;
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Vector accepts (Point, Point) arguments, (" +
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    return Context::RuntimeError
    {
        "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
        "Vector only suports 1 or 2 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        node.charPosition + node.content.length(),
        1
    };
}

RuntimeValue evaluateCrossFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)
{
    const std::array<int, 3>& cIdx{ node.children };

    if (args.size() == 2)
    {
        if (std::holds_alternative<Eval::Vector>(args[0]) &&
            std::holds_alternative<Eval::Vector>(args[1]))
        {
            const Eval::Vector& u{ std::get<Eval::Vector>(args[0]) };
            const Eval::Vector& v{ std::get<Eval::Vector>(args[1]) };

            Eval::Vector cross
            {
                glm::vec3{ 0.0f, 0.0f, 0.0f },
                glm::cross(u.head - u.origin, v.head - v.origin)
            };

            cross.pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
            cross.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

            return cross;
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Cross accepts (Vector, Vector) arguments, (" +
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    return Context::RuntimeError
    {
        "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
        "Cross only suports 2 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        node.charPosition + node.content.length(),
        1
    };
}

RuntimeValue evaluateSegmentFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)
{
    const std::array<int, 3>& cIdx{ node.children };

    if (args.size() == 2)
    {
        auto p0{ extractPoint(args[0]) };
        auto p1{ extractPoint(args[1]) };

        if (p0 && p1)
        {
            Eval::Segment segment{ *p0, *p1 };

            segment.pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
            segment.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

            return segment;
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Segment accepts (Point, Point) arguments, (" +
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    return Context::RuntimeError
    {
        "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
        "Segment only suports 2 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        node.charPosition + node.content.length(),
        1
    };
}

RuntimeValue evaluateLineFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)
{
    const std::array<int, 3>& cIdx{ node.children };

    if (args.size() == 2)
    {
        if (std::holds_alternative<Eval::Vector>(args[1]))
        {
            auto p{ extractPoint(args[0]) };

            if (p)
            {
                Eval::Vector vector{ std::get<Eval::Vector>(args[1]) };

                Eval::Line line{ *p, vector.origin, vector.head };

                line.pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
                line.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

                return line;
            }
        }

        else if (std::holds_alternative<glm::vec3>(args[1]) || std::holds_alternative<Eval::IPoint>(args[1]))
        {
            auto p0{ extractPoint(args[0]) };
            auto p1{ extractPoint(args[1]) };

            if (p0 && p1)
            {
                Eval::Vector vector{ *p0, *p1 };

                Eval::Line line{ *p0, vector.origin, vector.head };

                line.pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
                line.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

                return line;
            }
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Line accepts (Point, Vector), or (Point, Point) arguments, (" +
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    return Context::RuntimeError
    {
        "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
        "Line only suports 2 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        node.charPosition + node.content.length(),
        1
    };
}

RuntimeValue evaluatePlaneFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)
{
    const std::array<int, 3>& cIdx{ node.children };

    if (args.size() == 2)
    {
        if (std::holds_alternative<Eval::Vector>(args[1]))
        {
            auto p{ extractPoint(args[0]) };

            if (p)
            {
                Eval::Vector vector{ std::get<Eval::Vector>(args[1]) };

                Eval::Plane plane{ *p, vector.origin, vector.head };

                plane.pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
                plane.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

                return plane;
            }
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Plane accepts (Point, Vector) arguments, (" +
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    else if (args.size() == 3)
    {
        auto A{ extractPoint(args[0]) };
        auto B{ extractPoint(args[1]) };
        auto C{ extractPoint(args[2]) };

        if (A && B && C)
        {
            glm::vec3 u{ *B - *A };
            glm::vec3 v{ *C - *A };

            glm::vec3 normal{};

            const float lengthProduct{ glm::length(u) * glm::length(v) };
            constexpr float epsilon_0{ 0.001f };
            if (lengthProduct > epsilon_0)
            {
                const float theta{ glm::dot(u, v) / lengthProduct };

                constexpr float epsilon_1{ 0.999f };
                if (glm::abs(theta) > epsilon_1)
                {
                    glm::vec3 right{};

                    getNewCoordSystem(u, right, normal);
                }
                else
                {
                    normal = glm::cross(u, v);
                }
            }
            else
            {
                normal = glm::cross(u, v);
            }

            Eval::Plane plane{ *A, glm::vec3(0.0f, 0.0f, 0.0f), normal };

            plane.pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
            plane.pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);
            plane.pTypes[2] = deduceTypeByIdentifierName(nodes[cIdx[2]].content);

            return plane;
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };
        const Object::Type t2{ deduceTypeByIdentifierName(nodes[cIdx[2]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Plane accepts (Point, Point, Point) arguments, (" +
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ", " +
            getStringFunctionType(t2) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    return Context::RuntimeError
    {
        "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
        "Plane only suports 2 or 3 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        node.charPosition + node.content.length(),
        1
    };
}

RuntimeValue evaluateIntersectFunc(const std::vector<RuntimeValue>& args, const Node& node, const std::vector<Node>& nodes)
{
    const std::array<int, 3>& cIdx{ node.children };

    if (nodes[cIdx[0]].type != Node::Variable || nodes[cIdx[1]].type != Node::Variable)
    {
        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Intersect only suports variables as arguments.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    if (args.size() == 2)
    {
        if ((std::holds_alternative<Eval::Line>(args[0]) || std::holds_alternative<Eval::ILine>(args[0])) &&
            (std::holds_alternative<Eval::Line>(args[1]) || std::holds_alternative<Eval::ILine>(args[1])))
        {
            std::optional<Eval::Line> line0{ extractLine(args[0]) };
            std::optional<Eval::Line> line1{ extractLine(args[1]) };

            RuntimeValue temp{ intersectionLineLine(*line0, *line1) };

            if (Eval::IPoint* intersection{ std::get_if<Eval::IPoint>(&temp) })
            {
                intersection->pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
                intersection->pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

                return *intersection;
            }

            return temp;
        }

        else if ((std::holds_alternative<Eval::Line>(args[0]) || std::holds_alternative<Eval::ILine>(args[0])) &&
                 std::holds_alternative<Eval::Plane>(args[1]))
        {
            std::optional<Eval::Line> line0{ extractLine(args[0]) };

            RuntimeValue temp{ intersectionLinePlane(*line0, std::get<Eval::Plane>(args[1])) };

            if (Eval::IPoint * intersection{ std::get_if<Eval::IPoint>(&temp) })
            {
                intersection->pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
                intersection->pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

                return *intersection;
            }

            return temp;
        }

        else if (std::holds_alternative<Eval::Plane>(args[0]) &&
            (std::holds_alternative<Eval::Line>(args[1]) || std::holds_alternative<Eval::ILine>(args[1])))
        {
            std::optional<Eval::Line> line1{ extractLine(args[1]) };

            RuntimeValue temp{ intersectionLinePlane(*line1, std::get<Eval::Plane>(args[0])) };

            if (Eval::IPoint * intersection{ std::get_if<Eval::IPoint>(&temp) })
            {
                intersection->pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
                intersection->pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

                return *intersection;
            }

            return temp;
        }

        else if (std::holds_alternative<Eval::Plane>(args[0]) &&
                 std::holds_alternative<Eval::Plane>(args[1]))
        {
            RuntimeValue temp{ intersectionPlanePlane(std::get<Eval::Plane>(args[0]), std::get<Eval::Plane>(args[1])) };

            if (Eval::ILine* intersection{ std::get_if<Eval::ILine>(&temp) })
            {
                intersection->pTypes[0] = deduceTypeByIdentifierName(nodes[cIdx[0]].content);
                intersection->pTypes[1] = deduceTypeByIdentifierName(nodes[cIdx[1]].content);

                return *intersection;
            }

            return temp;
        }

        const Object::Type t0{ deduceTypeByIdentifierName(nodes[cIdx[0]].content) };
        const Object::Type t1{ deduceTypeByIdentifierName(nodes[cIdx[1]].content) };

        return Context::RuntimeError
        {
            "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
            "Intersect accepts (Line, Line), (Line, Plane), or\n(Plane, Plane) arguments, (" + 
            getStringFunctionType(t0) + ", " +
            getStringFunctionType(t1) + ") given.",
            Context::ErrorSeverity::Error,
            node.charPosition + node.content.length(),
            1
        };
    }

    return Context::RuntimeError
    {
        "Semantics Error at col " + std::to_string(node.charPosition + node.content.length() + 1) + ": "
        "Intersect only suports 2 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        node.charPosition + node.content.length(),
        1
    };
}

RuntimeValue evaluateIntersectFunc(const std::vector<RuntimeValue>& args)
{
    if (args.size() == 2)
    {
        if ((std::holds_alternative<Eval::Line>(args[0]) || std::holds_alternative<Eval::ILine>(args[0])) &&
            (std::holds_alternative<Eval::Line>(args[1]) || std::holds_alternative<Eval::ILine>(args[1])))
        {
            std::optional<Eval::Line> line0{ extractLine(args[0]) };
            std::optional<Eval::Line> line1{ extractLine(args[1]) };

            RuntimeValue temp{ intersectionLineLine(*line0, *line1) };

            if (Eval::IPoint * intersection{ std::get_if<Eval::IPoint>(&temp) })
            {
                intersection->pTypes[0] = Object::Line;
                intersection->pTypes[1] = Object::Line;

                return *intersection;
            }

            return temp;
        }

       else if ((std::holds_alternative<Eval::Line>(args[0]) || std::holds_alternative<Eval::ILine>(args[0])) &&
                 std::holds_alternative<Eval::Plane>(args[1]))
        {
            std::optional<Eval::Line> line0{ extractLine(args[0]) };

            RuntimeValue temp{ intersectionLinePlane(*line0, std::get<Eval::Plane>(args[1])) };

            if (Eval::IPoint * intersection{ std::get_if<Eval::IPoint>(&temp) })
            {
                intersection->pTypes[0] = Object::Line;
                intersection->pTypes[1] = Object::Plane;

                return *intersection;
            }

            return temp;
        }

       else if (std::holds_alternative<Eval::Plane>(args[0]) &&
           (std::holds_alternative<Eval::Line>(args[1]) || std::holds_alternative<Eval::ILine>(args[1])))
        {
            std::optional<Eval::Line> line1{ extractLine(args[1]) };

            RuntimeValue temp{ intersectionLinePlane(*line1, std::get<Eval::Plane>(args[0])) };

            if (Eval::IPoint * intersection{ std::get_if<Eval::IPoint>(&temp) })
            {
                intersection->pTypes[0] = Object::Plane;
                intersection->pTypes[1] = Object::Line;

                return *intersection;
            }

            return temp;
        }

        else if (args.size() == 2 &&
            std::holds_alternative<Eval::Plane>(args[0]) &&
            std::holds_alternative<Eval::Plane>(args[1])
            )
        {
            RuntimeValue temp{ intersectionPlanePlane(std::get<Eval::Plane>(args[0]), std::get<Eval::Plane>(args[1])) };

            if (Eval::ILine * intersection{ std::get_if<Eval::ILine>(&temp) })
            {
                intersection->pTypes[0] = Object::Plane;
                intersection->pTypes[1] = Object::Plane;

                return *intersection;
            }

            return temp;
        }
    }

    return Context::RuntimeError
    {
        "Semantics Error at col 10: "
        "Intersect only suports 2 arguments, " + std::to_string(args.size()) + " given.",
        Context::ErrorSeverity::Error,
        9,
        1
    };
}

Object::Type deduceTypeByIdentifierName(std::string_view func)
{
    if      (func == "Point") return Object::Point;
    else if (func == "Vector") return Object::Vector;
    else if (func == "Cross") return Object::Vector;
    else if (func == "Segment") return Object::Segment;
    else if (func == "Line") return Object::Line;
    else if (func == "Plane") return Object::Plane;
    else
    {
        int idx{ searchObjectIndexByName(func, Context::object) };

        if (idx >= 0)
        {
            return Context::object[idx].getType();
        }
    }

    if (convertSVToFloat(func))
    {
        return Object::Number;
    }

    return Object::Null;
}

std::array<int, 3> findParentsIDs(const std::vector<Node>& nodes)
{
    const std::array<int, 3>& childrenIdx{ nodes[0].children };
    std::array<int, 3> pIDs{ -1, -1, -1 };

    int i{ 0 };
    while (i < childrenIdx.size())
    {
        if (childrenIdx[i] != -1)
        {
            int idx{ searchObjectIndexByName(nodes[childrenIdx[i]].content, Context::object) };

            if (idx >= 0) pIDs[i] = Context::object[idx].getID();
        }
        else
        {
            pIDs[i] = Context::componentLiteral;
        }

        ++i;
    }

    return pIDs;
}
