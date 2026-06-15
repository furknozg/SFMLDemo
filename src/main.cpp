#include <SFML/Graphics.hpp>

#include <iostream>
#include <cstdlib>
#include <optional>
#include <typeindex>
#include <type_traits>
#include <random> // Required for modern random tools


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600



using EntityId = std::uint32_t;

// entity will have an arbitrary number of components,
// and each component will have an arbitrary number of systems
// that operate on it.
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
    std::unordered_map<EntityId, TransformComponent> m_transforms;
    std::unordered_map<EntityId, VelocityComponent> m_velocities;
    std::unordered_map<EntityId, RenderComponent> m_renders;

public:
    void addTransform(EntityId entity, TransformComponent component)
    {
        m_transforms.insert_or_assign(entity, std::move(component));
    }
    void addRender(EntityId entity, RenderComponent component)
    {
        m_renders.insert_or_assign(entity, std::move(component));
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

    RenderComponent &getRender(EntityId entity)
    {
        return m_renders.at(entity);
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

class WallCollisionSystem : public System {
    public:
    void update(float dt, Registry &registry) override
    {
        for (const auto &[entity, transform] : registry.transforms())
        {
            if (!registry.hasVelocity(entity))
            {
                continue;
            }

            auto &velocity = registry.getVelocity(entity);

            if (transform.position.x >= WINDOW_WIDTH || transform.position.x <= 0) {
                velocity.velocity.x *= -1;
            } 
            if (transform.position.y >= WINDOW_HEIGHT || transform.position.y <= 0) {
                velocity.velocity.y *= -1;
                
            } 
            
        }
    }
};

class RenderSystem
{
public:
    void update(sf::RenderWindow &window, Registry &registry)
    {
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

            window.draw(*render.drawable);
        }
    }
};

class RandomNumberGenerator {
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_real_distribution<float> dist; 
    public:
        RandomNumberGenerator(float min, float max):
            gen(rd()), dist(min,max)

        {
        }
        float getRand(){
            return dist(gen);
        }
};

int main()
{
    int entity_ctr = 0;
    sf::RenderWindow window(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
        "Text Based engine");

    sf::Font font;
    
    if (!font.openFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }
    RandomNumberGenerator rand(45.0f, 100.0f);

    Registry registry;
    Entity circle(0, "Circle");
    auto circleShape = std::make_shared<sf::CircleShape>(40.0f);
    circleShape->setFillColor(sf::Color::Green);
    registry.addVelocity(circle.id(), VelocityComponent{rand.getRand(), rand.getRand()});
    registry.addTransform(circle.id(), TransformComponent{2.0f, 2.0f});
    registry.addRender(circle.id(), RenderComponent{circleShape});
    

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
        wallCollisionSystem.update(dt,registry);

        
        window.clear();
        renderSystem.update(window, registry);
        window.display();
    }

    return EXIT_SUCCESS;
}