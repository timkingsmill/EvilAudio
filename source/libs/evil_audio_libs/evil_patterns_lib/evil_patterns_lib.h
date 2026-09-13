#pragma once

/**
 * @defgroup evil_patterns_lib Evil Patterns Library
 * @brief Reusable MVC and command pattern building blocks for Evil Audio.
 *
 * @details
 * The library provides MVC interfaces, command abstractions, and supporting
 * factories for wiring view, controller, and model components in a consistent
 * way across Evil Audio applications.
 *
 * @section evil_patterns_lib_usage Usage Notes
 * - Use the command factory to register and invoke commands.
 * - Prefer MVC factories to ensure compatible types and observers.
 */

// ------------ Namespace documentation and architecture diagrams ------------------ 
/**
 * @namespace evil::mvc
 * @brief Model-View-Controller framework types used by Evil Audio applications.
 *
 * Provides abstract interfaces and helper factories for wiring data models,
 * views, and controllers using a consistent architecture.
 
 * @section mvc_architecture_patterns MVC Architecture Patterns
 * 
 * The MVC architecture pattern is designed to separate concerns in application
 * development, allowing for modularity and testability. The model manages the
 * data and business logic, the view handles the presentation layer, and the
 * controller acts as an intermediary that processes user input and updates the model.
 * This separation allows developers to modify the user interface 
 * without affecting the underlying data management and vice versa.
 * 
 * In the context of Evil Audio, this pattern is particularly beneficial for
 * audio applications where the user interface may need to be updated frequently
 * based on user interactions or real-time audio processing events. By adhering to
 * the MVC pattern, developers can ensure that the application remains responsive and
 * maintainable, even as the complexity of the audio processing logic increases.
 *
 * @details
 * The namespace defines a lightweight MVC contract:
 * - Model: owns application state and emits model events.
 * - View: renders state and forwards user actions.
 * - Controller: validates user input and coordinates model updates.
 * 
 *  @plantumlfile evil_patterns_lib/MVCClassDiagram.puml
 * 
 * @section mvc_sequence_diagrams MVC Sequence Diagram
 * The sequence diagram illustrates the typical flow of user interactions 
 * through the MVC pipeline, including command execution and model updates.
 * 
 * @plantumlfile evil_patterns_lib/MVCSequenceDiagram.puml
 * 
 * @section mvc_namespace_usage Usage Notes
 * - Use makeModel, makeView, and makeController factory helpers to enforce type constraints.
 * - Register views as observers of the model when they react to model events.
 * - Keep domain validation in controllers and state mutation in models.
 */

// =========================================================================================

// ------------ File documentation and architecture diagrams ------------------ 
/**
 * @file evil_patterns_lib.h
 * @brief Aggregate include for the Evil Patterns library and architecture diagram.
 *
 * @details
 * Includes the public MVC contracts, command abstractions, and type/factory
 * helpers needed by consumers integrating the patterns library.
 *
 * @section generated_header_diagram Generated Header Diagram
 * The diagram below is generated from the project headers during the docs build.
 *
 */
        
// =========================================================================================

namespace evil
{
    namespace mvc
    {

    }
}









