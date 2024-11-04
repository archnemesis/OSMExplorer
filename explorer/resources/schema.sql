-- SCHEMA: osmexplorer

-- DROP SCHEMA IF EXISTS osmexplorer ;

CREATE SCHEMA IF NOT EXISTS osmexplorer
    AUTHORIZATION postgres;

-- Table: osmexplorer.layers

-- DROP TABLE IF EXISTS osmexplorer.layers;

CREATE TABLE IF NOT EXISTS osmexplorer.layers
(
    id uuid NOT NULL,
    created timestamp(0) without time zone NOT NULL,
    updated timestamp(0) without time zone,
    name character varying COLLATE pg_catalog."default" NOT NULL,
    description text COLLATE pg_catalog."default",
    "order" integer NOT NULL DEFAULT 0,
    locked boolean NOT NULL DEFAULT false,
    CONSTRAINT layers_pkey PRIMARY KEY (id)
    )

    TABLESPACE pg_default;

ALTER TABLE IF EXISTS osmexplorer.layers
    OWNER to postgres;

-- Table: osmexplorer.objects

-- DROP TABLE IF EXISTS osmexplorer.objects;

CREATE TABLE IF NOT EXISTS osmexplorer.objects
(
    id uuid NOT NULL,
    layer_id uuid NOT NULL,
    type character varying(128) COLLATE pg_catalog."default" NOT NULL,
    created timestamp(0) without time zone NOT NULL,
    updated timestamp(0) without time zone,
    label character varying(256) COLLATE pg_catalog."default" NOT NULL,
    description text COLLATE pg_catalog."default",
    geom geometry NOT NULL,
    data jsonb NOT NULL,
    CONSTRAINT objects_pkey PRIMARY KEY (id)
    )

    TABLESPACE pg_default;

ALTER TABLE IF EXISTS osmexplorer.objects
    OWNER to postgres;