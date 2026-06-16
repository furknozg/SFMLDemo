#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <optional>
#include <typeindex>
#include <type_traits>
#include <random> // Required for modern random tools
#include <map>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

using EntityId = std::uint32_t;

// I used an ECS-style layout here so the data for a shape is separated from
// the logic that acts on it. An entity is just an id, and the Registry connects
// that id to whatever components the shape needs.
// entity will have an arbitrary number of components,
// and each component will have an arbitrary number of systems
// that could operate on it.
// The entity will be responsible for managing the components and systems,
// and will provide an interface for adding, removing, and accessing them.
struct Component
{
    virtual ~Component() = default;
};

struct TransformComponent : public Component
{
    sf::Vector2f position;

    TransformComponent(float x, float y)
        : position(x, y)
    {
    }
};
struct VelocityComponent : public Component
{
    sf::Vector2f velocity;

    VelocityComponent(float x, float y)
        : velocity(x, y)
    {
    }
};

struct Material
{
    sf::Color fillColor = sf::Color::White;
    sf::Color outlineColor = sf::Color::Transparent;
    float outlineThickness = 0.0f;
};

struct RenderComponent : public Component
{
    std::shared_ptr<sf::Drawable> drawable;
    Material material;
    RenderComponent(std::shared_ptr<sf::Drawable> drawable, Material material = {})
        : drawable(std::move(drawable)),
          material(material)
    {
    }
};

class Registry
{
    // Each map stores one kind of component, keyed by the entity id.
    // If an entity has entries in multiple maps, the systems can combine them.
    std::unordered_map<EntityId, TransformComponent> m_transforms;
    std::unordered_map<EntityId, VelocityComponent> m_velocities;
    std::unordered_map<EntityId, RenderComponent> m_renders;
    std::unordered_map<EntityId, RenderComponent> m_textRenders;

public:
    void addTransform(EntityId entity, TransformComponent component)
    {
        m_transforms.insert_or_assign(entity, std::move(component));
    }
    void addRender(EntityId entity, RenderComponent component)
    {
        m_renders.insert_or_assign(entity, std::move(component));
    }
    void addTextRender(EntityId entity, RenderComponent component)
    {
        m_textRenders.insert_or_assign(entity, std::move(component));
    }

    void addVelocity(EntityId entity, VelocityComponent component)
    {
        m_velocities.insert_or_assign(entity, std::move(component));
    }

    bool hasTransform(EntityId entity) const
    {
        return m_transforms.find(entity) != m_transforms.end();
    }
    bool hasRender(EntityId entity) const
    {
        return m_renders.find(entity) != m_renders.end();
    }
    bool hasVelocity(EntityId entity) const
    {
        return m_velocities.find(entity) != m_velocities.end();
    }
    bool hasTextRender(EntityId entity) const
    {
        return m_textRenders.find(entity) != m_renders.end();
    }
    RenderComponent &getRender(EntityId entity)
    {
        return m_renders.at(entity);
    }
    RenderComponent &getTextRender(EntityId entity)
    {
        return m_textRenders.at(entity);
    }
    VelocityComponent &getVelocity(EntityId entity)
    {
        return m_velocities.at(entity);
    }

    TransformComponent &getTransform(EntityId entity)
    {
        return m_transforms.at(entity);
    }

    const std::unordered_map<EntityId, RenderComponent> &renders() const
    {
        return m_renders;
    }
    const std::unordered_map<EntityId, RenderComponent> &textRenders() const
    {
        return m_textRenders;
    }

    const std::unordered_map<EntityId, VelocityComponent> &velocities() const
    {
        return m_velocities;
    }

    const std::unordered_map<EntityId, TransformComponent> &transforms() const
    {
        return m_transforms;
    }
};

class Entity
{
private:
    EntityId m_id;
    std::string m_tag;

public:
    Entity(EntityId id, std::string tag)
        : m_id(id), m_tag(std::move(tag))
    {
    }

    EntityId id() const
    {
        return m_id;
    }

    const std::string &tag() const
    {
        return m_tag;
    }
};

class System
{
public:
    virtual ~System() = default;
    virtual void update(float dt, Registry &registry) = 0;
};

class MovementSystem : public System
{
public:
    void update(float dt, Registry &registry) override
    {
        // Move every entity that has both velocity and position.
        // Multiplying by dt keeps movement based on time instead of frame count.
        for (const auto &[entity, velocity] : registry.velocities())
        {
            if (!registry.hasTransform(entity))
            {
                continue;
            }

            auto &transform = registry.getTransform(entity);

            transform.position += velocity.velocity * dt;
        }
    }
};

class WallCollisionSystem
{
public:
    void update(float dt, int window_width, int window_height, Registry &registry)
    {
        // Check the rendered shape size so circles and rectangles can bounce
        // using the same collision rule.
        for (const auto &[entity, transform] : registry.transforms())
        {
            if (!registry.hasVelocity(entity))
            {
                continue;
            }

            auto &velocity = registry.getVelocity(entity);
            auto &render = registry.getRender(entity);

            auto *circle = dynamic_cast<sf::CircleShape *>(render.drawable.get());
            auto *rect = dynamic_cast<sf::RectangleShape *>(render.drawable.get());

            float w_x = 0;
            float w_y = 0;
            if (circle)
            {
                w_x = 2 * circle->getRadius();
                w_y = 2 * circle->getRadius();
            }

            if (rect)
            {
                sf::Vector2f r_size = rect->getSize();
                w_x = r_size.x;
                w_y = r_size.y;
            }

            if (transform.position.x + w_x >= window_width || transform.position.x <= 0)
            {
                velocity.velocity.x *= -1;
            }
            if (transform.position.y + w_y >= window_height || transform.position.y <= 0)
            {
                velocity.velocity.y *= -1;
            }
        }
    }
};

class RenderSystem
{
    void applyMaterial(const RenderComponent &render)
    {
        // RenderComponent stores sf::Drawable so different drawable types can
        // be kept together. Material only applies to sf::Shape subclasses.
        auto *shape = dynamic_cast<sf::Shape *>(render.drawable.get());
        if (!shape)
        {
            // try to get text if render is not a shape
            auto *text = dynamic_cast<sf::Text *>(render.drawable.get());
            if (!shape)
            {
                std::cerr << "ERROR: RenderComponent is not an instance of Shape or Text" << std::endl;
            }

            text->setFillColor(render.material.fillColor);
        }

        shape->setFillColor(render.material.fillColor);
        shape->setOutlineColor(render.material.outlineColor);
        shape->setOutlineThickness(render.material.outlineThickness);
    }

public:
    void update(sf::RenderWindow &window, Registry &registry)
    {
        // Rendering is the final system. It copies component data back onto
        // SFML objects, then draws whatever drawable is stored for the entity.
        for (const auto &[entity, render] : registry.renders())
        {
            if (!registry.hasTransform(entity))
            {
                continue;
            }

            auto &transform = registry.getTransform(entity);

            auto transformable =
                dynamic_cast<sf::Transformable *>(render.drawable.get());

            if (transformable)
            {
                transformable->setPosition(transform.position);
            }

            if (registry.hasTextRender(entity))
            {
                auto &text = registry.getTextRender(entity);
                auto transformableText =
                    dynamic_cast<sf::Transformable *>(text.drawable.get());
                if (transformableText)
                {
                    transformableText->setPosition(transform.position);
                }
                applyMaterial(text);
                window.draw(*text.drawable);
            }


            applyMaterial(render);
            window.draw(*render.drawable);
        }
    }
};

class RandomNumberGenerator
{
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist;

public:
    RandomNumberGenerator(float min, float max) : gen(rd()), dist(min, max)

    {
    }
    float getRand()
    {
        return dist(gen);
    }
};

enum class ConfigStatementType
{
    WINDOW,
    FONT,
    ENTITY_LIKE
};
enum class EntityType
{
    RECTANGLE,
    CIRCLE
};

struct FontConfig
{
    std::string filePath;
    int size;
    std::vector<int> RGB = {0, 0, 0};
};

class ConfigLoader
{
    std::string m_filePath;
    Registry *m_registry;
    int m_window_width;
    int m_window_height;
    FontConfig m_font_config;
    int m_lineIndex = 0;

    std::vector<std::string> splitWords(const std::string &line)
    {
        std::stringstream ss(line);

        std::vector<std::string> words;
        std::string word;

        while (ss >> word)
        {
            words.push_back(word);
        }

        return words;
    }

    EntityType getEntityType(const std::string &line)
    {
        static const std::map<std::string, EntityType> typeMap = {
            {"Circle", EntityType::CIRCLE},
            {"Rectangle", EntityType::RECTANGLE},
        };
        auto it = typeMap.find(line);
        if (it != typeMap.end())
        {
            return it->second;
        }
        std::cout << "Warn: Entity type not in configuration loader. Defaulting to ENTITY_LIKE" << std::endl;
        return EntityType::CIRCLE;
    }

    void parseAndLoadEntity(const std::string &line, int index)
    {
        std::vector<std::string> words = splitWords(line);

        EntityType type = getEntityType(words[0]);
        // Shape lines all start the same way:
        // Type name initialPosition(x,y) initialSpeed(x,y) color(r,g,b)
        // Circles then have radius, while rectangles have width and height.
        std::string shapeName = words[1];
        TransformComponent position(std::stof(words[2]), std::stof(words[3]));
        VelocityComponent velocity(std::stof(words[4]), std::stof(words[5]));
        Material material = {
            fillColor : sf::Color(std::stoi(words[6]), std::stoi(words[7]), std::stoi(words[8]))
        };
        RenderComponent *rc;

        // Add the shared components first. The shape-specific render component
        // gets added below after I know whether this line is a circle or rect.
        m_registry->addVelocity(m_lineIndex, velocity);
        m_registry->addTransform(m_lineIndex, position);

        if (type == EntityType::CIRCLE)
        {
            Entity circle(m_lineIndex, shapeName);
            auto circleShape = std::make_shared<sf::CircleShape>(std::stof(words[9]));
            RenderComponent rc = RenderComponent(circleShape, material);

            m_registry->addRender(m_lineIndex, rc);
        }
        else if (type == EntityType::RECTANGLE)
        {
            sf::Vector2f size = {std::stof(words[9]), std::stof(words[10])};
            Entity rect(m_lineIndex, shapeName);
            auto rectShape = std::make_shared<sf::RectangleShape>(size);
            RenderComponent rc = RenderComponent(rectShape, material);

            m_registry->addRender(m_lineIndex, rc);
        }
    };

    ConfigStatementType getStatementType(const std::string &str)
    {
        static const std::map<std::string, ConfigStatementType> statementMap = {
            {"Window", ConfigStatementType::WINDOW},
            {"Font", ConfigStatementType::FONT},
            {"Circle", ConfigStatementType::ENTITY_LIKE},
            {"Square", ConfigStatementType::ENTITY_LIKE},
        };
        auto it = statementMap.find(str);
        if (it != statementMap.end())
        {
            return it->second;
        }
        std::cout << "Warn: Entity type not in configuration loader. Defaulting to ENTITY_LIKE" << std::endl;
        return ConfigStatementType::ENTITY_LIKE;
    };

    void parserHandler(std::string line)
    {
        std::vector<std::string> words = splitWords(line);
        ConfigStatementType type = getStatementType(words[0]);

        switch (type)
        {

        case ConfigStatementType::WINDOW:
            m_window_width = std::stoi(words[1]);
            m_window_height = std::stoi(words[2]);
            break;
        case ConfigStatementType::FONT:
            // 5 args
            m_font_config = {
                filePath : words[1],
                size : std::stoi(words[2]),
                RGB : {std::stoi(words[3]), std::stoi(words[4]), std::stoi(words[5])}
            };

            break;
        case ConfigStatementType::ENTITY_LIKE:
            parseAndLoadEntity(line, m_lineIndex);
            break;
        default:
            break;
        }
    };

public:
    ConfigLoader(std::string filePath) : m_filePath(filePath)
    {
    }
    void LoadEntities(Registry *data)
    {
        m_registry = data;
        std::ifstream file(m_filePath);
        if (!file)
        {
            std::cerr << "Could not open file\n";
            return;
        }
        std::string line;
        while (std::getline(file, line))
        {
            parserHandler(line);
            m_lineIndex++;
        }
    }
    std::vector<unsigned int> getWindow()
    {
        return {(unsigned int)m_window_width, (unsigned int)m_window_height};
    }
    sf::Font getFont()
    {
        sf::Font font;

        if (!font.openFromFile(m_font_config.filePath))
        {
            return font;
        }

        return font;
        // R
    }
};

int main()
{
    // Load config into the registry before creating the game loop. After this,
    // main only has to run the systems in order every frame.
    Registry registry;
    ConfigLoader config("config.txt");
    config.LoadEntities(&registry);

    std::vector<unsigned int> dims = config.getWindow();
    sf::RenderWindow window(
        sf::VideoMode({dims[0], dims[1]}),
        "Text Based engine");

    sf::Font font;

    if (!font.openFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }
    // RandomNumberGenerator rand(-400.0f, 400.0f);

    MovementSystem movementSystem;
    RenderSystem renderSystem;
    WallCollisionSystem wallCollisionSystem;

    sf::Clock clock;

    // event loop/render loop
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        movementSystem.update(dt, registry);
        wallCollisionSystem.update(dt, dims[0], dims[1], registry);

        window.clear();
        renderSystem.update(window, registry);
        window.display();
    }

    return EXIT_SUCCESS;
}
