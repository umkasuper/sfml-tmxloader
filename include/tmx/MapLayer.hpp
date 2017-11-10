/*********************************************************************
Matt Marchant 2013 - 2016
SFML Tiled Map Loader - https://github.com/bjorn/tiled/wiki/TMX-Map-Format
						http://trederia.blogspot.com/2013/05/tiled-map-loader-for-sfml.html

Zlib License:

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.

2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#ifndef MAPLAYER_HPP_
#define MAPLAYER_HPP_

#include <tmx/MapObject.hpp>
#include <tmx/Export.hpp>

#include <memory>
#include <array>

namespace tmx
{
    class LayerSet;
	class TMX_EXPORT_API TileQuad final
	{
		friend class LayerSet;
	public:
		using Ptr = std::shared_ptr<TileQuad>; //TODO shared libs don't like this being a unique_ptr
		TileQuad(sf::Uint16 i0, sf::Uint16 i1, sf::Uint16 i2, sf::Uint16 i3);
		void move(const sf::Vector2f& distance);
        void setVisible(bool);
	private:
		std::array<sf::Uint16, 4u> m_indices;
        sf::Color m_colour;
		sf::Vector2f m_movement;
		LayerSet* m_parentSet;
		sf::Int32 m_patchIndex;
        void setDirty();
	};

	/*!
    \brief Drawable composed of vertices representing a set of tiles on a layer
    */
	class TMX_EXPORT_API LayerSet final : public sf::Drawable
	{
		friend class TileQuad;
	public:

        // структура для описания фрейма
        class TileFrameDescription {
        private:
            const sf::Uint16 m_Duration; // Длительность показа
            const sf::Uint16 m_tileId;   // ID tile который показывать
        public:
            TileFrameDescription(const sf::Uint16 duration, const sf::Uint16 tileID) :
                    m_Duration(duration),
                    m_tileId(tileID) {}
        };

        class TileFrame {
        private:
            const TileFrameDescription m_frameDescription;
        public:
            const sf::Texture& m_texture;
            const sf::Texture &getTexture() {return m_texture;}
            TileFrame(const TileFrameDescription &frameDescription, const sf::Texture& texture) :
                m_frameDescription(frameDescription),
                m_texture(texture) {}
        };

        enum class TileType
        {
            Tile,
            AnimationTile
        };

        struct TileInfo
        {
            TileType m_type;
            std::array<sf::Vector2f, 4> Coords;
            std::vector<TileFrameDescription> m_tileFrames;
            sf::Vector2f Size;
            sf::Uint16 TileSetId; // индекс в массиве m_tilesetTextures
            sf::Uint16 m_tileId; // ID в xml
            TileInfo();
            TileInfo(const sf::IntRect& rect, const sf::Vector2f& size, sf::Uint16 tilesetId, sf::Uint16 teleId, const std::vector<TileFrameDescription> &tileFrames);
        };

		LayerSet(sf::Uint8 patchSize, const sf::Vector2u& mapSize, const sf::Vector2u tileSize, std::vector<TileFrame> &tileFrame, LayerSet::TileInfo &tileInfo);
		TileQuad* addTile(sf::Vertex vt0, sf::Vertex vt1, sf::Vertex vt2, sf::Vertex vt3, sf::Uint16 x, sf::Uint16 y);
		void cull(const sf::FloatRect& bounds);


	private:
        const std::vector<TileFrame> m_tileFrame; // массив кадров
		const TileInfo& m_tileInfo; // ссылка на TileInfo
        const sf::Uint8 m_patchSize;
		const sf::Vector2u m_mapSize;
		const sf::Vector2u m_patchCount;
		const sf::Vector2u m_tileSize;

		std::vector<TileQuad::Ptr> m_quads;
		mutable std::vector<TileQuad*> m_dirtyQuads;

		sf::Vector2i m_visiblePatchStart, m_visiblePatchEnd;
		mutable std::vector<std::vector<sf::Vertex>> m_patches;

		void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;

		mutable sf::FloatRect m_boundingBox;
		void updateAABB(sf::Vector2f position, sf::Vector2f size);
		bool m_visible;    // виден или нет объект

        int m_frameNumber; // Номер кадра из m_tileFrame, для отрисовки анимации
	};



	/*!
    \brief Used to query the type of layer, for example when looking for layers containing collision objects
    */
	enum MapLayerType
	{
		Layer,
		ObjectGroup,
		ImageLayer
	};

	/*!
    \brief Represents a layer of tiles, corresponding to a tmx layer, object group or image layer
    */
	class TMX_EXPORT_API MapLayer final : public sf::Drawable
	{
	public:
		//used for drawing specific layers
		enum DrawType
		{
			Front,
			Back,
			Debug,
			All
		};


		explicit MapLayer(MapLayerType layerType);
		std::string name;
		float opacity; //range 0 - 1
		bool visible;
		MapTiles tiles;
		MapObjects objects; //vector of objects if layer is object group
		MapLayerType type;
		std::map <std::string, std::string> properties;

		std::map<sf::Uint16, std::shared_ptr<LayerSet>> layerSets;
        /*!
        \brief Sets the shader which will be used when drawing this layer
        */
		void setShader(const sf::Shader& shader);
        /*!
        \brief Used to cull patches outside the visible area
        */
		void cull(const sf::FloatRect& bounds);

	private:
		const sf::Shader* m_shader;
		void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
	};
};

#endif //MAPLAYER_HPP_