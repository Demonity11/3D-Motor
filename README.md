# 3[D] Motor

A high-performance, real-time 3D Geometric Evaluation & Rendering Engine built in Modern C++20 and OpenGL. Inspired by parametric geometric tools such as GeoGebra 3D, this project provides an interactive environment for dynamically defining, parsing, and rendering 3D spatial primitives.

---

## 🏛️ System Architecture

The engine decouples user command processing from GPU graphics execution through a four-stage compiler frontend connected to a dual-pass rendering pipeline:

```text
 +------------------+     +-------------------+     +-------------------+     +-------------------+
 |  ImGui Console   | --> | Lexical Analyzer  | --> | Stratified Parser | --> |   AST Evaluator   |
 |   Command Line   |     |    std::string    |     | std::vector<Node> |     |   RuntimeValue    |
 +------------------+     +-------------------+     +-------------------+     +-------------------+
                                                                                    |
                                                                                    v
 +------------------+     +-------------------+     +-------------------+     +-------------------+
 |   GPU Shader     | <-- |  Dual-Pass Queue  | <-- | Selection & Scene | <-- | Global Context    |
 | glDrawArrays Call|     | Opaque/Transparent|     | m_isVisible Flag  |     | Context::object   |
 +------------------+     +-------------------+     +-------------------+     +-------------------+

```

* **Lexical Analyzer (Lexer):** Scans command strings into tokens using non-allocating `std::string_view` slices, handling negative numbers, trailing dots, and recording column offsets for exact diagnostic tracking.
* **Stratified Recursive Descent Parser:** Implements operator precedence (`Exp1` for `+`/`-`, `Exp2` for `*`) to construct an acyclic, contiguous Abstract Syntax Tree (AST) in a `std::vector<Node>`. Supports nested expression parsing inside function parameters (e.g., `Point(1 + 2, a * 5, 3)`), chained operators, and assignment l-value validation.
* **AST Evaluator:** Traverses the AST bottom-up, resolving runtime types via polymorphic `std::variant<float, glm::vec3, Eval::Vector, Eval::IPoint>` containers (`RuntimeValue`). Enforces strict Affine Geometry algebra rules, dot products, and scalar variable bindings.
* **Selection & Visibility Manager:** Leverages a fast $O(1)$ `m_isVisible` property per primitive to bypass hidden geometry during draw calls and raycasting hit tests, using reverse-pass iteration to prioritize recently created overlapping primitives.
* **Dual-Pass Renderer:** Measures primitive distances relative to camera space, sorting objects into distinct opaque and transparent render queues to ensure blending accuracy.

---

## 🛠️ Key Engine Features

### 1. Embedded Scripting Engine & Diagnostic Feedback

* **Dynamic DSL Processing:** Evaluates complex geometric commands, algebraic expressions, and scalar/primitive variable bindings in real time.
* **Column-Exact Error Reporting:** Highlights precise character offsets in the ImGui terminal for lexical, syntactic, or semantic type errors (e.g., invalid operator combinations or unassigned standalone scalar expressions).

### 2. Stratified Precedence & Algebraic Expression Engine

* **Operator Precedence Parsing:** Handles arithmetic hierarchy (`*` evaluated before `+`/`-`) through precedence climbing without needing token stream reordering.
* **Nested Parameter Expressions:** Evaluates algebraic expressions directly within function calls, allowing dynamic primitive generation like `Vector(Point(1,1,1) + u)`.
* **Scalar Variable Bindings:** Supports scalar variables (`float`), enabling dot product results (`k = u * v`) or numeric multipliers (`k = 5`) to be stored in the global context for scaling primitives.

### 3. Affine Geometry & Semantic Type System

* **Type-Safe Spatial Operations:** Strict semantic evaluation enforcing mathematically valid affine space operations:
* **Translation:** $\text{Point} \pm \text{Vector} \rightarrow \text{Point}$
* **Displacement:** $\text{Point} - \text{Point} \rightarrow \text{Vector}$
* **Vector Addition:** $\text{Vector} \pm \text{Vector} \rightarrow \text{Vector}$
* **Scaling:** $\text{Scalar} \times \text{Vector} \rightarrow \text{Vector}$ or $\text{Scalar} \times \text{Point} \rightarrow \text{Point}$
* **Dot Product:** $\text{Vector} \times \text{Vector} \rightarrow \text{Scalar}$ (via `glm::dot`)


* **Semantic Error Interception:** Automatically rejects geometric non-sense like $\text{Point} + \text{Point}$ or unassigned standalone scalar values.

### 4. Visibility Control & Reverse-Pass Mouse Selection

* **$O(1)$ Render Filtering:** Primitives contain a boolean `m_isVisible` flag, allowing instantly hiding overlapping geometry without mutating or deleting scene graph memory.
* **Reverse Raycasting Selection:** Mouse picking iterates backward through the spatial object registry ($N-1 \rightarrow 0$), guaranteeing that newly created or overlapping geometry is selected first during viewport clicks.

### 5. Dual-Pass Transparency & OIT Mitigation

To solve depth-buffer fighting and flickering when rendering overlapping $2\text{D}$ transparent planes ($\alpha = 0.2$):

* **Opaque Pass ($\alpha = 1.0$):** Renders $0\text{D}$ points and $1\text{D}$ lines/vectors with Depth Testing enabled and `glDepthMask(GL_TRUE)`. Objects are sorted **Front-to-Back** to optimize GPU early-Z rejection.
* **Transparent Pass ($\alpha < 1.0$):** Renders transparent planes sorted strictly **Back-to-Front** relative to camera distance with depth buffer writing disabled via `glDepthMask(GL_FALSE)`.

---

## ⌨️ Command API & Expression Matrix

### Function Commands

| Function Command | Parameter Signatures | Derived Geometric Type | Description |
| --- | --- | --- | --- |
| `Point` | `Point(x, y, z)` | `Point` ($0\text{D}$) | Creates a point literal at coordinates $(x, y, z)$. Accepts nested expressions for coordinates. |
| `Vector` | `Vector(P)` <br> <br> `Vector(P1, P2)` | `Vector` ($1\text{D}$) | Creates a directed vector from the origin to $P$, or spanning $P_1 \rightarrow P_2$. |
| `Segment` | `Segment(P1, P2)` | `Segment` ($1\text{D}$) | Creates a bounded segment between points $P_1$ and $P_2$. |
| `Line` | `Line(P, V)` <br> <br> `Line(P1, P2)` | `Line` ($1\text{D}$) | Creates an infinite line passing through $P$ along direction $V$ or connecting $P_1$ and $P_2$. |
| `Plane` | `Plane(P, V)` <br> <br> `Plane(A, B, C)` | `Plane` ($2\text{D}$) | Constructs an infinite surface from a point and normal vector, or from 3 points. |
| `Cross` | `Cross(V1, V2)` | `Vector` ($1\text{D}$) | Calculates the vector cross product $\mathbf{V}_1 \times \mathbf{V}_2$. |
| `Intersect` | `Intersect(Obj1, Obj2)` | `Point` / `Line` | Evaluates analytic intersections between Line-Line, Line-Plane, or Plane-Plane. |

### Supported Algebraic Operators

| Operator | Left Operand | Right Operand | Resulting Type | Semantic Meaning |
| --- | --- | --- | --- | --- |
| `+` | `Scalar` | `Scalar` | `Scalar` | Scalar addition |
| `+` | `Vector` | `Vector` | `Vector` | Vector addition ($\mathbf{u} + \mathbf{v}$) |
| `+` | `Point` | `Vector` | `Point` | Point translation ($P + \mathbf{v}$) |
| `+` | `Vector` | `Point` | `Point` | Point translation ($\mathbf{v} + P$) |
| `-` | `Scalar` | `Scalar` | `Scalar` | Scalar subtraction |
| `-` | `Vector` | `Vector` | `Vector` | Vector subtraction ($\mathbf{u} - \mathbf{v}$) |
| `-` | `Point` | `Point` | `Vector` | Displacement vector from $B$ to $A$ ($A - B$) |
| `-` | `Point` | `Vector` | `Point` | Inverse point translation ($P - \mathbf{v}$) |
| `*` | `Scalar` | `Scalar` | `Scalar` | Scalar multiplication |
| `*` | `Scalar` | `Vector` | `Vector` | Vector scaling ($k \cdot \mathbf{v}$) |
| `*` | `Vector` | `Scalar` | `Vector` | Vector scaling ($\mathbf{v} \cdot k$) |
| `*` | `Scalar` | `Point` | `Point` | Position scaling relative to origin |
| `*` | `Point` | `Scalar` | `Point` | Position scaling relative to origin |
| `*` | `Vector` | `Vector` | `Scalar` | **Dot Product** ($\mathbf{u} \cdot \mathbf{v}$) |

---

## 📸 Media & Demos

### Main Interface

*(Screenshot placeholder: Interactive 3D Viewport with Coordinate Axes and ImGui Terminal)*

### Interactive Command Console & Real-Time Parsing

*(GIF placeholder: Executing script commands, defining variables, and live visual update)*

### Plane Intersections & Transparency Rendering

*(GIF/Screenshot placeholder: Multi-plane blending with Z-buffer stability)*

---

## 🏗️ Building the Project

*(This section will be updated with build instructions, application icons, and platform-specific configurations in an upcoming project release).*

---

## ✍️ Author's Note

Building **3[D] Motor** has been an incredible milestone in my journey as a Computer Science student aiming to specialize in graphics programming and software architecture.

Moving from foundational C++ concepts to designing a custom language interpreter and an OpenGL renderer forced me to connect mathematical theory with low-level computing. Implementing linear algebra concepts, working out spatial projections, handling transparency sorting, and building an interactive frontend with Dear ImGui presented non-trivial engineering challenges that significantly deepened my understanding of systems architecture.

There is always more to refine and build, and I am excited to continue expanding my knowledge in low-level graphics API development.

---

## 📚 Credits & References

* **learncpp.com:** Fundamental C++ techniques and core modern language paradigms.
* **learnopengl.com:** Mathematical intuition for vector algebra, projections, and spatial transformations.
* **GeoGebra:** Functional inspiration for interactive parametric geometric modeling tools.
* **Dear ImGui & GLFW:** GUI framework and windowing abstraction layers.
