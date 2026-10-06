# Machine Learning Example: Recommendation Using Vectors

Each user is represented as a vector of preferences.
Each product is represented as a vector of features.
The dot product is used to measure how well a product matches a user.

## Example

Let the feature set be:

- smart
- 4k
- gaming
- portable

Now represent users and products as numeric vectors:

### Users

- User 1 = [1, 1, 1, 0]
- User 2 = [1, 0, 1, 1]
- User 3 = [0, 1, 0, 1]
- User 4 = [1, 1, 0, 0]

### Products

- Smart TV = [1, 1, 0, 0]
- Mobile = [1, 0, 1, 1]
- iPhone = [1, 1, 1, 1]
- Tablet = [0, 1, 0, 1]

## Dot product formula

score(user, product) = user · product

Example:

- User 1 · Smart TV = (1×1) + (1×1) + (1×0) + (0×0) = 2
- User 1 · Mobile = (1×1) + (1×0) + (1×1) + (0×1) = 2
- User 1 · iPhone = (1×1) + (1×1) + (1×1) + (0×1) = 3

So User 1 matches iPhone the most.

## How ML works here

1. Data is converted into numbers.
2. A model learns patterns from those numbers.
3. Similar items are compared using similarity scores.
4. The item with the highest score is recommended.

This is a simple ML idea used in recommendation systems.

## Simple explanation

Machine learning is not magic. It means:

- collect data
- turn data into features
- train a model on examples
- predict or recommend based on similarity

In this example, the model compares the user vector with product vectors using the dot product.

