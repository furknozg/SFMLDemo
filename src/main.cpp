#include <SFML/Graphics.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Context
{
    int m_width = 0;
    int m_height = 0;

public:
    void setWidth(int width)
    {
        m_width = width;
    }

    void setHeight(int height)
    {
        m_height = height;
    }

    int getWidth() const
    {
        return m_width;
    }

    int getHeight() const
    {
        return m_height;
    }
};

class DrawableShape
{
    std::shared_ptr<sf::Shape> m_shape;
    std::shared_ptr<sf::Text> m_name;
    sf::Vector2f m_speed;

public:
    DrawableShape(
        std::shared_ptr<sf::Shape> shape,
        std::shared_ptr<sf::Text> name,
        sf::Vector2f speed
    )
        : m_shape(std::move(shape))
        , m_name(std::move(name))
        , m_speed(speed)
    {
    }

    std::shared_ptr<sf::Shape> getShape() const
    {
        return m_shape;
    }

    std::shared_ptr<sf::Text> getName() const
    {
        return m_name;
    }

    void update(const Context& context)
    {
        sf::FloatRect bounds = m_shape->getGlobalBounds();

        if (
            bounds.position.x <= 0.0f ||
            bounds.position.x + bounds.size.x >= static_cast<float>(context.getWidth())
        )
        {
            m_speed.x *= -1.0f;
        }

        if (
            bounds.position.y <= 0.0f ||
            bounds.position.y + bounds.size.y >= static_cast<float>(context.getHeight())
        )
        {
            m_speed.y *= -1.0f;
        }

        sf::Vector2f shapePos = m_shape->getPosition();
        shapePos += m_speed;

        m_shape->setPosition(shapePos);

        const sf::FloatRect shapeBounds = m_shape->getLocalBounds();
        const sf::FloatRect textBounds = m_name->getLocalBounds();

        const sf::Vector2f shapeCenter{
            shapeBounds.position.x + shapeBounds.size.x / 2.0f,
            shapeBounds.position.y + shapeBounds.size.y / 2.0f
        };

        const sf::Vector2f textCenter{
            textBounds.position.x + textBounds.size.x / 2.0f,
            textBounds.position.y + textBounds.size.y / 2.0f
        };

        m_name->setPosition(shapePos + shapeCenter - textCenter);
    }
};

int main()
{
    sf::RenderWindow window;
    Context context;

    sf::Font font;
    sf::Color fontColor = sf::Color::White;
    unsigned int fontSize = 24;

    int windowWidth = 0;
    int windowHeight = 0;

    std::ifstream fin("config.txt");

    if (!fin.is_open())
    {
        std::cerr << "Could not open config.txt\n";
        return EXIT_FAILURE;
    }

    std::vector<std::shared_ptr<DrawableShape>> shapes;

    std::string token;

    while (fin >> token)
    {
        if (token == "Font")
        {
            std::string fontFilename;
            int r = 0;
            int g = 0;
            int b = 0;

            fin >> fontFilename >> fontSize >> r >> g >> b;

            if (!font.openFromFile(fontFilename))
            {
                std::cerr << "Could not load font: " << fontFilename << '\n';
                return EXIT_FAILURE;
            }

            fontColor = sf::Color(
                static_cast<std::uint8_t>(r),
                static_cast<std::uint8_t>(g),
                static_cast<std::uint8_t>(b)
            );
        }
        else if (token == "Window")
        {
            fin >> windowWidth >> windowHeight;

            window.create(
                sf::VideoMode({
                    static_cast<unsigned int>(windowWidth),
                    static_cast<unsigned int>(windowHeight)
                }),
                "COMP 4300 - Assignment 1"
            );

            context.setWidth(windowWidth);
            context.setHeight(windowHeight);
        }
        else if (token == "Rectangle")
        {
            std::string shapeName;

            float initPosX = 0.0f;
            float initPosY = 0.0f;
            float initSpeedX = 0.0f;
            float initSpeedY = 0.0f;

            int r = 0;
            int g = 0;
            int b = 0;

            float rectWidth = 0.0f;
            float rectHeight = 0.0f;

            fin >> shapeName
                >> initPosX >> initPosY
                >> initSpeedX >> initSpeedY
                >> r >> g >> b
                >> rectWidth >> rectHeight;

            auto rectangle = std::make_shared<sf::RectangleShape>(
                sf::Vector2f{rectWidth, rectHeight}
            );

            rectangle->setFillColor(sf::Color(
                static_cast<std::uint8_t>(r),
                static_cast<std::uint8_t>(g),
                static_cast<std::uint8_t>(b)
            ));

            rectangle->setPosition({initPosX, initPosY});

            auto text = std::make_shared<sf::Text>(font, shapeName, fontSize);
            text->setFillColor(fontColor);

            shapes.push_back(std::make_shared<DrawableShape>(
                rectangle,
                text,
                sf::Vector2f{initSpeedX, initSpeedY}
            ));
        }
        else if (token == "Circle")
        {
            std::string shapeName;

            float initPosX = 0.0f;
            float initPosY = 0.0f;
            float initSpeedX = 0.0f;
            float initSpeedY = 0.0f;

            int r = 0;
            int g = 0;
            int b = 0;

            float circleRadius = 0.0f;

            fin >> shapeName
                >> initPosX >> initPosY
                >> initSpeedX >> initSpeedY
                >> r >> g >> b
                >> circleRadius;

            auto circle = std::make_shared<sf::CircleShape>(circleRadius);

            circle->setFillColor(sf::Color(
                static_cast<std::uint8_t>(r),
                static_cast<std::uint8_t>(g),
                static_cast<std::uint8_t>(b)
            ));

            circle->setPosition({initPosX, initPosY});

            auto text = std::make_shared<sf::Text>(font, shapeName, fontSize);
            text->setFillColor(fontColor);

            shapes.push_back(std::make_shared<DrawableShape>(
                circle,
                text,
                sf::Vector2f{initSpeedX, initSpeedY}
            ));
        }
        else
        {
            std::cerr << "Unknown config token: " << token << '\n';
        }
    }

    if (!window.isOpen())
    {
        std::cerr << "Window was not created. Check config.txt for a Window entry.\n";
        return EXIT_FAILURE;
    }

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        for (const auto& shape : shapes)
        {
            shape->update(context);
        }

        window.clear();

        for (const auto& shape : shapes)
        {
            window.draw(*shape->getShape());
            window.draw(*shape->getName());
        }

        window.display();
    }

    return EXIT_SUCCESS;
}