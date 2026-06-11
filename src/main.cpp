#include <SFML/Graphics.hpp>

#include <iostream>
#include <cstdlib>
#include <optional>

enum Shape2D
{
    SHAPE_CIRCLE,
    SHAPE_RECTANGLE
};



class Transform2D
{
    sf::Vector2f m_position;
    sf::Angle m_rotation;
    sf::Vector2f m_scale;

public:
    Transform2D(const sf::Transformable &data)
        : m_position(data.getPosition()), m_rotation(data.getRotation()), m_scale(data.getScale())
    {
    }
};

class Text2D
{
    std::string m_text = "Sample Text";
    sf::Font m_font;
    Transform2D m_transform;
    sf::Text m_sfText = sf::Text(m_font, m_text); // SFML text object for rendering
public:
    Text2D(const std::string &text, const sf::Font &font, const Transform2D *transform)
        : m_text(text), m_font(font), m_transform(*transform)
    {
        m_sfText.setFont(m_font);
        m_sfText.setString(m_text);
    }

    void draw(sf::RenderWindow &window)
    {
        // draw text
        // Implementation for drawing text
        window.draw(m_sfText);
    }
};


class Drawable2D // our main entity-like class.
{

    std::string m_name = "Drawable2D";
    Transform2D m_transform;
    Shape2D m_shape;
    

    void drawCircle()
    {
        // draw circle
    }
    void drawRectangle()
    {
        // draw rectangle
    }

public:
    Drawable2D(const std::string &name, const Transform2D *transform, const Shape2D shape)
        : m_name(name), m_transform(*transform), m_shape(shape)
    {
    }

    void draw(sf::RenderWindow &window)
    {
        switch (m_shape)
        {
        case SHAPE_CIRCLE:
            // draw circle
            drawCircle();
            break;
        case SHAPE_RECTANGLE:
            // draw rectangle
            drawRectangle();
            break;
        default:
            std::cerr << "Unknown shape type: " << m_shape << std::endl;
            break;
        }
    }
};


class Entity
{
    std::string m_name = "Entity";
    Drawable2D m_drawable;
    Text2D m_text;

public:
    Entity(const std::string &name, const Drawable2D *drawable, const Text2D *text)
        : m_name(name), m_drawable(*drawable), m_text(*text)
    {
    }

    void update()
    {
    }
};




int main()
{
    sf::RenderWindow window(
        sf::VideoMode({800, 600}),
        "Single Circle");

    sf::Font font;
    if (!font.openFromFile("fonts/arial.ttf"))
    {
        return EXIT_FAILURE;
    }

    sf::CircleShape circle(50.0f);
    circle.setFillColor(sf::Color::Green);
    circle.setPosition({350.0f, 250.0f});

    Text2D text("Circle", font, nullptr);
    text.setFillColor(sf::Color::White);

    const sf::FloatRect circleBounds = circle.getLocalBounds();
    const sf::FloatRect textBounds = text.getLocalBounds();

    const sf::Vector2f circleCenter{
        circleBounds.position.x + circleBounds.size.x / 2.0f,
        circleBounds.position.y + circleBounds.size.y / 2.0f};

    const sf::Vector2f textCenter{
        textBounds.position.x + textBounds.size.x / 2.0f,
        textBounds.position.y + textBounds.size.y / 2.0f};

    text.setPosition(circle.getPosition() + circleCenter - textCenter);

    // event loop/render loop
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear(sf::Color::Black);
        window.draw(circle);
        window.draw(text);
        window.display();
    }

    return EXIT_SUCCESS;
}